#include "correlation_core_pulsar.h"
#include "output_header.h"
#include <utils.h>

Correlation_core_pulsar::Correlation_core_pulsar(): nbins(0), polyco(NULL){
 us_per_day=(int64_t)24*60*60*1000000;
}

Correlation_core_pulsar::~Correlation_core_pulsar() {
}

void
Correlation_core_pulsar::set_parameters(const Correlation_parameters &parameters,
                                        Pulsar &pulsar,
                                        std::vector<Delay_table_akima> &delays,
                                        std::vector<std::vector<double> > &uvw,
                                        int node_nr) {
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;
  delay_tables = delays;
  uvw_table = uvw;

  // If the relevant correlation parameters change, clear the window
  // vector.  It will be recreated when the next integration starts.
  if (parameters.number_channels != correlation_parameters.number_channels ||
      parameters.fft_size_correlation != correlation_parameters.fft_size_correlation ||
      parameters.window != correlation_parameters.window) {
    window.clear();
    mask.clear();
  }

  correlation_parameters = parameters;
  if (correlation_parameters.mask_parameters)
    mask_parameters = *correlation_parameters.mask_parameters;

  create_baselines(parameters);
  if (input_elements.size() != number_input_streams()) {
    input_elements.resize(number_input_streams());
  }
  if (input_conj_buffers.size() != number_input_streams()) {
    input_conj_buffers.resize(number_input_streams());
  }
  n_flagged.resize(baselines.size());

  double start_mjd = parameters.start_time.get_mjd();
  fft_duration = ((double)fft_size() * 1000000) / parameters.sample_rate;
  if (offsets.size() != fft_size() + 1)
    offsets.resize(fft_size() + 1);
  if (bins.size() != fft_size() + 1)
    bins.resize(fft_size() + 1);

  // Find the appropiate polyco
  nbins = pulsar.nbins + 1; // Extra bin for off-pulse data
  double diff = std::abs(pulsar.polyco_params[0].tmid-start_mjd);
  int closest=0;
  for(int i=1; i<pulsar.polyco_params.size(); i++){
    double new_diff=std::abs(pulsar.polyco_params[i].tmid-start_mjd);
    if(new_diff<diff){
      diff=new_diff;
      closest = i;
    }
  }
  polyco = &pulsar.polyco_params[closest];

  // Compute the phase at the start of the period
  double DT;
  if (parameters.window == SFXC_WINDOW_NONE)
    DT = ((start_mjd - polyco->tmid) + fft_duration/(2*us_per_day))*1440;
  else
    DT = ((start_mjd - polyco->tmid) + fft_duration/us_per_day)*1440;
  int N = polyco->coef.size();
  start_phase = 0;
  for (int i=N-1; i>0; i--){
    start_phase = (start_phase + polyco->coef[i])*DT;
  }
  start_phase += (polyco->ref_phase-floor(polyco->ref_phase))+DT*60*polyco->ref_freq + polyco->coef[0]; 

  // Find the time offsets between frequency components
  int sb = parameters.sideband == 'L' ? -1 : 1;
  double base_freq = parameters.channel_freq*1e-6; // [MHZ]
  double dfreq = sb * parameters.sample_rate * 1e-6/ ( 2 * fft_size());
  // TODO check accuracy
  double inv_freq_obs2 = 1/(polyco->obs_freq*polyco->obs_freq);
  double freq = 0;
  for(int i=N-1;i>1;i--)
    freq = (freq + i*polyco->coef[i])*DT;
  freq = (freq + polyco->coef[1]) / 60 + polyco->ref_freq;

  SFXC_ASSERT(offsets.size() == fft_size() + 1);
  for(int i = 0; i < fft_size() + 1 ;i++) {
    offsets[i] = 4148.808 * polyco->DM * (1 / pow(base_freq + i * dfreq, 2) - inv_freq_obs2) * freq;
  }
  gate.begin = pulsar.interval.start;
  gate.end = pulsar.interval.stop;

  if(accumulation_buffers.size()!=nbins)
    accumulation_buffers.resize(nbins);
}

void Correlation_core_pulsar::do_task() {
  SFXC_ASSERT(has_work());

  if (current_fft % 1000 == 0) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);
  }

  if (current_fft % number_ffts_in_integration == 0) {
    integration_initialise();
  }

  for (size_t i = 0; i < number_input_streams(); i++) {
    int stream = station_stream(i);
    input_elements[i] = &input_buffers[stream]->front()->data[0];
    if (input_buffers[stream]->front()->data.size() > input_conj_buffers[i].size())
      input_conj_buffers[i].resize(input_buffers[stream]->front()->data.size());
  }
  const int first_stream = station_stream(0);
  const int stride = input_buffers[first_stream]->front()->stride;
  const int nbuffer = input_buffers[first_stream]->front()->data.size() / stride;
  for (size_t buf_idx = 0; buf_idx < nbuffer * stride; buf_idx += stride) {
    // Process the data of the current fft
    integration_step(dedispersion_buffer, buf_idx);
    dedisperse_buffer();
    current_fft++;
  }

  for (size_t i = 0; i < number_input_streams(); i++) {
    int stream = station_stream(i);
    input_buffers[stream]->pop();
  }

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    find_invalid();
    for(int bin = 0; bin < nbins; bin++) {
      integration_normalize(accumulation_buffers[bin]);
      integration_write(accumulation_buffers[bin], 0, bin);
    }
    tsys_write();
    current_integration++;
  }
}

void Correlation_core_pulsar::integration_initialise() {
  const int size = fft_size() + 1;
  for(int bin = 0; bin < nbins; bin++) {
    if (accumulation_buffers[bin].size() != baselines.size()) {
      accumulation_buffers[bin].resize(baselines.size());
      for (int j = 0; j < accumulation_buffers[bin].size(); j++) {
        accumulation_buffers[bin][j].resize(size);
      }
    }
  }
  if (dedispersion_buffer.size() != baselines.size()) {
    dedispersion_buffer.resize(baselines.size());
    for (int j = 0; j < dedispersion_buffer.size(); j++) {
      dedispersion_buffer[j].resize(size);
    }
  }

  for (int bin = 0; bin < nbins; bin++) {
    for (int j = 0; j < accumulation_buffers[bin].size(); j++) {
      SFXC_ASSERT(accumulation_buffers[bin][j].size() == size);
      memset(&accumulation_buffers[bin][j][0], 0, size * sizeof(std::complex<FLOAT>));
    }
  }

  for (int j = 0; j < dedispersion_buffer.size(); j++) {
    SFXC_ASSERT(dedispersion_buffer[j].size() == size);
    memset(&dedispersion_buffer[j][0], 0, size * sizeof(std::complex<FLOAT>));
  }
  memset(&n_flagged[0], 0, sizeof(std::pair<int64_t,int64_t>)*n_flagged.size());
  fft_f2t.resize(2 * fft_size());
  fft_t2f.resize(2 * number_channels());
  temp_buffer.resize(fft_size() + 1);
  real_buffer.resize(2 * fft_size());

  if (fft_size() != number_channels()){
    create_mask();
    create_window();
  }
}

void Correlation_core_pulsar::integration_step(std::vector<Complex_buffer> &integration_buffer, int buf_idx) {
#ifndef DUMMY_CORRELATION
  // Auto correlations
  for (size_t i = 0; i < number_input_streams(); i++) {
    // get the complex conjugates of the input
    SFXC_CONJ_FC(&input_elements[i][buf_idx], &(input_conj_buffers[i])[0], fft_size() + 1);
    SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[i][buf_idx], 
                        /* in2 */ &input_conj_buffers[i][0],
                        /* out */ &integration_buffer[i][0], fft_size() + 1);
  }
  
  // Cross correlations
  for (size_t i = number_input_streams(); i < baselines.size(); i++) {
    std::pair<size_t, size_t> &baseline = baselines[i];
    SFXC_ASSERT(baseline.first != baseline.second);
    SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[baseline.first][buf_idx], 
                        /* in2 */ &input_conj_buffers[baseline.second][0],
                        /* out */ &integration_buffer[i][0], fft_size() + 1);
  }
#endif // DUMMY_CORRELATION
}

void Correlation_core_pulsar::dedisperse_buffer() {
  double obs_freq_phase = get_phase();
  double len=gate.end-gate.begin;
  // first compute the phase bins
  SFXC_ASSERT(bins.size() == fft_size() + 1);
  for (int j = 0; j < fft_size() + 1; j++) {
    double phase = obs_freq_phase - offsets[j];
    phase = phase - floor(phase);
    if (phase >= gate.begin){
      if(phase < gate.end)
        bins[j] = (int)((phase-gate.begin)*(nbins-1)/len) + 1;
      else
        bins[j] = 0;
    }else if (phase + 1 < gate.end){
      bins[j] = (int)((phase + 1 - gate.begin)*(nbins-1)/len) + 1;
    }else
      bins[j] = 0;
  }

  // TODO check performance agains loop interchange
  for (int i = 0; i < baselines.size(); i++) {
    SFXC_ASSERT(dedispersion_buffer[i].size() == fft_size() + 1);
    for(int j = 0 ; j < fft_size() + 1; j++) {
      int bin = bins[j];
      accumulation_buffers[bin][i][j] += dedispersion_buffer[i][j];
      dedispersion_buffer[i][j]=0;
    }
  }
}
