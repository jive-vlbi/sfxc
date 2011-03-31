#include "correlation_core_pulsar.h"
#include "output_header.h"
#include <utils.h>

Correlation_core_pulsar::Correlation_core_pulsar(): nbins(0), polyco(NULL){
 us_per_day=(int64_t)24*60*60*1000000;
#ifdef SFXC_WRITE_STATS
    sfxc_abort("SFXC_WRITE_STATS currently doesn't work with pulsar binning.\n");
#endif // SFXC_WRITE_STATS
}

Correlation_core_pulsar::~Correlation_core_pulsar() {
}

void
Correlation_core_pulsar::set_parameters(const Correlation_parameters &parameters,
                                 int node_nr) {
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;

  correlation_parameters = parameters;
  oversamp = (int) round(parameters.sample_rate / (2 * parameters.bandwidth));

  create_baselines(parameters);
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  n_flagged.resize(baselines.size());

  double start_mjd = parameters.start_time.get_mjd();
  fft_duration = ((double)fft_size() * 1000000) / parameters.sample_rate;
  if (offsets.size() != fft_size() + 1)
    offsets.resize(fft_size() + 1);
  if (bins.size() != fft_size() + 1)
    bins.resize(fft_size() + 1);

  pulsar_params = parameters.pulsar_parameters;
  std::map<std::string, Pulsar_parameters::Pulsar>::iterator cur_pulsar_it =
                           pulsar_params->pulsars.find(std::string(&parameters.source[0]));

  if (cur_pulsar_it==pulsar_params->pulsars.end()){
    // When correlating a non-pulsar (calibrator) source we set nbins=1 and set the
    // pulsar gate equal to the pulsar period 
    if(!polyco)
      polyco = &pulsar_params->pulsars.begin()->second.polyco_params[0];
    nbins=1;
    gate.begin=0;
    gate.end=1;
  }else{
    // Find the appropiate polyco
    Pulsar_parameters::Pulsar &cur_pulsar = cur_pulsar_it->second;
    nbins = cur_pulsar.nbins;
    double diff = std::abs(cur_pulsar.polyco_params[0].tmid-start_mjd);
    int closest=0;
    for(int i=1; i<cur_pulsar.polyco_params.size(); i++){
      double new_diff=std::abs(cur_pulsar.polyco_params[i].tmid-start_mjd);
      if(new_diff<diff){
        diff=new_diff;
        closest = i;
      }
    }
    polyco = &cur_pulsar.polyco_params[closest];

    // Compute the phase at the start of the period
    double DT=((start_mjd - polyco->tmid) + fft_duration/(2*us_per_day))*1440;
    start_phase = (polyco->ref_phase-floor(polyco->ref_phase))+DT*60*polyco->ref_freq + polyco->coef[0]; 
    for (int i=1; i<polyco->coef.size(); i++){
      start_phase += polyco->coef[i]*pow(DT,i);
    }

    // Find the time offsets between frequency components
    int sb = parameters.sideband == 'L' ? -1 : 1;
    double base_freq = parameters.channel_freq*1e-6; // [MHZ]
    double dfreq = sb * parameters.sample_rate * 1e-6/ ( 2 * fft_size());
    // TODO check accuracy
    double inv_freq_obs2 = 1/(polyco->obs_freq*polyco->obs_freq);
    double freq = polyco->ref_freq;
    for(int i=1;i<polyco->n_coef;i++)
      freq += i*pow(DT,i-1)*polyco->coef[i]/60;

    SFXC_ASSERT(offsets.size() == fft_size() + 1);
    for(int i = 0; i < fft_size() + 1 ;i++) {
      offsets[i] = 4148.808 * polyco->DM * (1 / pow(base_freq + i * dfreq, 2) - inv_freq_obs2) * freq;
    }
    gate.begin = cur_pulsar.interval.start;
    gate.end = cur_pulsar.interval.stop;
  }

  if(accumulation_buffers.size()!=nbins)
    accumulation_buffers.resize(nbins);
}

void Correlation_core_pulsar::do_task() {
  SFXC_ASSERT(has_work());
  if (current_fft % 1000 == 0) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);
  }

  if (current_fft%number_ffts_in_integration == 0) {
    integration_initialise();
  }

  SFXC_ASSERT(input_buffers.size()==number_input_streams_in_use());
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    input_elements[i] = &input_buffers[i]->front()->data[0];
  }
  find_invalid();
  const int stride = input_buffers[0]->front()->stride;
  const int nbuffer = input_buffers[0]->front()->data.size() / stride;
  for (int buf = 0; buf < nbuffer * stride ; buf += stride){
    // Process the data of the current fft
    integration_step(dedispersion_buffer, buf);
    dedisperse_buffer();
    current_fft ++;
  }

  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++)
    input_buffers[i]->pop();

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    for(int bin=0;bin<nbins;bin++){
      integration_normalize(accumulation_buffers[bin]);
      integration_write(accumulation_buffers[bin], 0, bin);
    }
    current_integration++;
  }
}

void Correlation_core_pulsar::integration_initialise() {
  const int size = fft_size() + 1;
  for(int bin=0;bin<nbins;bin++){
    if (accumulation_buffers[bin].size() != baselines.size()) {
      accumulation_buffers[bin].resize(baselines.size());
      for (int j=0; j<accumulation_buffers[bin].size(); j++) {
        accumulation_buffers[bin][j].resize(size);
      }
    }
  }
  if (dedispersion_buffer.size() != baselines.size()) {
    dedispersion_buffer.resize(baselines.size());
    for (int j=0; j<dedispersion_buffer.size(); j++) {
      dedispersion_buffer[j].resize(size);
    }
  }

  for (int bin=0; bin<nbins; bin++) {
    for (int j=0; j<accumulation_buffers[bin].size(); j++) {
      SFXC_ASSERT(accumulation_buffers[bin][j].size() == size);
      memset(&accumulation_buffers[bin][j][0], 0, size*sizeof(std::complex<FLOAT>));
    }
  }

  for (int j=0; j<dedispersion_buffer.size(); j++) {
    SFXC_ASSERT(dedispersion_buffer[j].size() == size);
    memset(&dedispersion_buffer[j][0], 0, size*sizeof(std::complex<FLOAT>));
  }
  memset(&n_flagged[0], 0, sizeof(std::pair<int64_t,int64_t>)*n_flagged.size());
}


void Correlation_core_pulsar::dedisperse_buffer() {
  double obs_freq_phase = get_phase();
  double len=gate.end-gate.begin;
  // first compute the phase bins
  SFXC_ASSERT(bins.size() == fft_size() + 1);
  for (int j=0; j < fft_size() + 1; j++) {
    double phase = obs_freq_phase+offsets[j];
    double dph = (phase-floor(phase))-gate.begin;
    if((dph>=0)&&(dph<=len)){
      bins[j] = (int)(dph*nbins/len);
    }else
      bins[j]=-1;
  }

  // TODO check performance agains loop interchange
  for (int i=0; i < baselines.size(); i++) {
    SFXC_ASSERT(dedispersion_buffer[i].size() == fft_size() + 1);
    for(int j= 0 ; j < fft_size() + 1; j++) {
      int bin = bins[j];
      if(bin >= 0){
        accumulation_buffers[bin][i][j] += dedispersion_buffer[i][j];
      }
      dedispersion_buffer[i][j]=0;
    }
  }
}
