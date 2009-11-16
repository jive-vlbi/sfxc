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

  size_t prev_size_of_fft = size_of_fft();
  correlation_parameters = parameters;

  create_baselines(parameters);

  double ms_per_day = 1000*24*60*60;
  double start_mjd = parameters.start_time/ms_per_day + parameters.mjd;
  fft_duration = (((double)n_channels())*1000000)/correlation_parameters.sample_rate;

  if(offsets.size()!=size_of_fft()/2+1)
    offsets.resize(size_of_fft()/2+1);
  if(bins.size() != size_of_fft()/2+1)
    bins.resize(size_of_fft()/2+1);

  pulsar_params = parameters.pulsar_parameters;
  std::map<std::string, Pulsar_parameters::Pulsar>::iterator cur_pulsar_it =
                           pulsar_params->pulsars.find(std::string(&parameters.source[0]));

  if (cur_pulsar_it==pulsar_params->pulsars.end()){
    // When correlating a non-pulsar (calibrator) source we set then nbins=1 and set the
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

    // Find the time offsets between frequency components
    int sb = correlation_parameters.sideband == 'L' ? -1 : 1;
    double base_freq = (parameters.channel_freq + sb*parameters.bandwidth*0.5)*1e-9; // [GHZ]
    double dfreq = parameters.bandwidth*1e-9/(n_channels()+1);
    // TODO check accuracy
    double ref_offset = 1/pow(polyco->obs_freq/1000,2);
    double freq = (polyco->n_coef>=2)?polyco->ref_freq+polyco->coef[1]/60:polyco->ref_freq;
    double period = 1000/freq;// in [ms]

    for(int i=0;i<size_of_fft()/2+1;i++){
      offsets[i] = 4.149*polyco->DM*(1/pow(base_freq+i*dfreq,2)-ref_offset)/period;
    }

    // Compute the phase at the start of the period
    double DT=(start_mjd+fft_duration/(2*us_per_day) - polyco->tmid)*1440;
    start_phase = (polyco->ref_phase-floor(polyco->ref_phase))+DT*60*polyco->ref_freq + polyco->coef[0];
    for (int i=1; i<polyco->coef.size(); i++){
      start_phase += polyco->coef[i]*pow(DT,i);
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

  // Process the data of the current fft
  integration_step(dedispersion_buffer);
  dedisperse_buffer();
  current_fft ++;

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    for(int bin=0;bin<nbins;bin++){
      integration_normalize(accumulation_buffers[bin]);
      integration_write(accumulation_buffers[bin]);
    }
    current_integration++;
  }
}

void Correlation_core_pulsar::integration_initialise() {
  int size = (size_of_fft()/2+1);
  for(int bin=0;bin<accumulation_buffers.size();bin++){
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

  for (int bin=0; bin<accumulation_buffers.size(); bin++) {
    for (int j=0; j<accumulation_buffers[bin].size(); j++) {
      SFXC_ASSERT(accumulation_buffers[bin][j].size() == size);
      memset(&accumulation_buffers[bin][j][0], size*sizeof(std::complex<FLOAT>), 0);
    }
  }
  for (int j=0; j<dedispersion_buffer.size(); j++) {
    SFXC_ASSERT(dedispersion_buffer[j].size() == size_of_fft()/2+1);
    memset(&dedispersion_buffer[j][0], size*sizeof(std::complex<FLOAT>), 0);
  }
}


void Correlation_core_pulsar::dedisperse_buffer() {
  double ref_phase = get_phase();
  double len=gate.end-gate.begin;
  // first compute the phase bins
  for(int j=0;j<size_of_fft()/2+1;j++){
    double phase = ref_phase+offsets[j];
    double dph = (phase-floor(phase)-gate.begin);
    if((dph>=0)&&(dph<=len))
      bins[j] = (int)(dph*nbins/len);
    else
      bins[j]=-1;
  }

  // TODO check performance agains loop interchange
  for (int i=0; i < baselines.size(); i++) {
    for(int j=0;j<size_of_fft()/2+1;j++){
      int bin = bins[j];
      if(bin >= 0){
        accumulation_buffers[bin][i][j] += dedispersion_buffer[i][j];
      }
    }
  }
}
