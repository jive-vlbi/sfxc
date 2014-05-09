#include "correlation_core_phased.h"
#include "output_header.h"
#include "bandpass.h"
#include "utils.h"
#include <set>

Correlation_core_phased::Correlation_core_phased()
{
}

Correlation_core_phased::~Correlation_core_phased() {
}

void Correlation_core_phased::do_task() {
  SFXC_ASSERT(has_work());
  if (current_fft % 1000 == 0) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);
  }

  if (current_fft%number_ffts_in_integration == 0) {
    integration_initialise();
  }
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    int j = streams_in_scan[i];
    input_elements[i] = &input_buffers[j]->front()->data[0];
    if (input_buffers[j]->front()->data.size() > input_conj_buffers[i].size())
      input_conj_buffers[i].resize(input_buffers[j]->front()->data.size());
  }
  const int first_stream = streams_in_scan[0];
  const int stride = input_buffers[first_stream]->front()->stride;
  const int nbuffer = input_buffers[first_stream]->front()->data.size() / stride;
  // Process the data of the current fft buffer
  int buf_idx=0;
  while((buf_idx < nbuffer) && (current_fft < number_ffts_in_integration)){
    int n = std::min(next_sub_integration*number_ffts_in_sub_integration - current_fft,
                     nbuffer - buf_idx);
    integration_step(accumulation_buffers, buf_idx, buf_idx+n, stride);
    current_fft += n;
    buf_idx += n;
    if (current_fft == number_ffts_in_integration) {
      PROGRESS_MSG("node " << node_nr_ << ", "
                   << current_fft << " of " << number_ffts_in_integration);

      sub_integration();
      for(int i = 0 ; i < phase_centers.size(); i++){
        int source_nr;
        if(split_output){
          delay_tables[0].goto_scan(correlation_parameters.start_time);
          source_nr = sources[delay_tables[0].get_source(i)];
        }else{
          source_nr = 0;
        }
        integration_write_headers(i, source_nr);
        integration_write_subints(phase_centers[i]);
      }
      current_integration++;
    }else if(current_fft >= next_sub_integration*number_ffts_in_sub_integration){
      sub_integration();
      next_sub_integration++;
    }
  }
  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++){
    int stream = streams_in_scan[i];
    input_buffers[stream]->pop();
  }
 
}

void
Correlation_core_phased::set_parameters(const Correlation_parameters &parameters,
                                 int node_nr) {
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;

  correlation_parameters = parameters;
  oversamp = (int) round(parameters.sample_rate / (2 * parameters.bandwidth));
  use_autocorrelations = parameters.only_autocorrelations;

  create_baselines(parameters);
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  if (input_conj_buffers.size() != number_input_streams_in_use()) {
    input_conj_buffers.resize(number_input_streams_in_use());
  }
  // Read calibration tables
  if (old_fft_size != fft_size()){
    old_fft_size = fft_size();
    if (cltable_name != std::string())
      cltable.open_table(cltable_name, fft_size());
    if (bptable_name != std::string())
      bptable.open_table(bptable_name, fft_size(), true);
  }
  n_flagged.resize(baselines.size());
  get_input_streams();
}

void Correlation_core_phased::integration_initialise() {
  number_output_products =
    ceil(number_ffts_in_integration / number_ffts_in_sub_integration);
  previous_fft = 0;

  if(phase_centers.size() != correlation_parameters.n_phase_centers)
    phase_centers.resize(correlation_parameters.n_phase_centers);

  for(int i = 0 ; i < phase_centers.size() ; i++){
    if (phase_centers[i].size() != number_output_products){
      phase_centers[i].resize(number_output_products);
      for(int j = 0 ; j < phase_centers[i].size() ; j++){
        phase_centers[i][j].resize(fft_size() + 1);
      }
    }
  }

  for(int i = 0 ; i < phase_centers.size() ; i++){
    for (int j = 0; j < phase_centers[i].size(); j++) {
      SFXC_ASSERT(phase_centers[i][j].size() == fft_size() + 1);
      size_t size = phase_centers[i][j].size() * sizeof(std::complex<FLOAT>);
      memset(&phase_centers[i][j][0], 0, size);
    }
  }

  if (accumulation_buffers.size() != baselines.size()) {
    accumulation_buffers.resize(baselines.size());
    for (size_t i=0; i<accumulation_buffers.size(); i++) {
      accumulation_buffers[i].resize(fft_size() + 1);
    }
  }

  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());
  for (size_t i=0; i<accumulation_buffers.size(); i++) {
    SFXC_ASSERT(accumulation_buffers[i].size() == fft_size() + 1);
    size_t size = accumulation_buffers[i].size() * sizeof(std::complex<FLOAT>);
    memset(&accumulation_buffers[i][0], 0, size);
  }
  memset(&n_flagged[0], 0, sizeof(std::pair<int64_t,int64_t>)*n_flagged.size());
  next_sub_integration = 1;
}

void Correlation_core_phased::
integration_step(std::vector<Complex_buffer> &integration_buffer, 
                 int first, int last, int stride) 
{
  for (size_t i = 0; i < number_input_streams_in_use(); i++) {
    for (size_t buf_idx = first*stride; 
         buf_idx < last * stride; 
         buf_idx += stride) {
      // get the complex conjugates of the input
      SFXC_CONJ_FC(&input_elements[i][buf_idx], &input_conj_buffers[i][buf_idx], 
                   fft_size() + 1);
    }
  }
  int begin, end;
  if (use_autocorrelations){
    begin = 0;
    end = number_input_streams_in_use();
  }else{
    begin = number_input_streams_in_use();
    end = baselines.size();
  }
  for (size_t i = begin; i < end; i++) {
    for (size_t buf_idx = first*stride; 
         buf_idx < last * stride; 
         buf_idx += stride) {
      // Cross correlations
      std::pair<size_t,size_t> &stations = baselines[i];
      //SFXC_ASSERT(stations.first != stations.second);
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[stations.first][buf_idx], 
			  /* in2 */ &input_conj_buffers[stations.second][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
    }
  }
}

void Correlation_core_phased::integration_write_subints(std::vector<Complex_buffer> &integration_buffer) {
  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());

  int polarisation = (correlation_parameters.polarisation == 'R')? 0 : 1;

  std::vector<float> float_buffer;
  float_buffer.resize(number_channels() + 1);
  Output_header_baseline hbaseline;

  size_t n = fft_size() / number_channels();
  for (size_t i = 0; i < integration_buffer.size(); i++) {
    for (size_t j = 0; j < number_channels(); j++) {
      float_buffer[j] = integration_buffer[i][j * n].real();
      for (size_t k = 1; k < n ; k++)
        float_buffer[j] += integration_buffer[i][j * n + k].real();
      float_buffer[j] /= n;
    }
    float_buffer[number_channels()] = 
                            integration_buffer[i][number_channels()*n].real();
    hbaseline.weight = 1;                        // The number of good samples
    hbaseline.station_nr1 = 0;
    hbaseline.station_nr2 = 0;

    // Polarisation for the first station
    SFXC_ASSERT((polarisation == 0) || (polarisation == 1)); // (RCP: 0, LCP: 1)
    hbaseline.polarisation1 = polarisation;
    hbaseline.polarisation2 = polarisation;
    // Upper or lower sideband (LSB: 0, USB: 1)
    if (correlation_parameters.sideband=='U') {
      hbaseline.sideband = 1;
    } else {
      SFXC_ASSERT(correlation_parameters.sideband == 'L');
      hbaseline.sideband = 0;
    }
    // The number of the channel in the vex-file,
    hbaseline.frequency_nr = (unsigned char)correlation_parameters.frequency_nr;
    // sorted increasingly
    // 1 byte left:
    hbaseline.empty = ' ';

    int nWrite = sizeof(hbaseline);
    writer->put_bytes(nWrite, (char *)&hbaseline);
    writer->put_bytes((number_channels() + 1) * sizeof(float),
                      ((char*)&float_buffer[0]));
  }
}

void 
Correlation_core_phased::sub_integration(){
  Time tfft(0., correlation_parameters.sample_rate); 
  tfft.inc_samples(fft_size());
  // Apply calibration
  const Time tmid = correlation_parameters.start_time + 
                    tfft*(previous_fft+(current_fft-previous_fft)/2.); 
  calibrate(accumulation_buffers, tmid); 

  const int n_fft = fft_size() + 1;
  const int n_phase_centers = phase_centers.size();
  const int n_station = number_input_streams_in_use();
  const int n_baseline = accumulation_buffers.size();
  if(RANK_OF_NODE ==-10) std::cout << "nbaseline = " << n_baseline
                                   << ", subint = " << next_sub_integration-1
                                   << " / " << phase_centers[0].size()
                                   << "fft = " << current_fft
                                   << ", nfft_per_sub="<< number_ffts_in_sub_integration
                                   << ", tmid = " << (int64_t)tmid.get_time_usec()
                                   << "\n";
  int begin, end;
  if (use_autocorrelations){
    begin = 0;
    end = n_station;
  }else{
    begin = n_station;
    end = n_baseline;
  }
  for(int i = begin ; i < end ; i++){
    std::pair<size_t,size_t> &inputs = baselines[i];
    int station1 = streams_in_scan[inputs.first];
    int station2 = streams_in_scan[inputs.second];

    // The pointing center
    SFXC_ADD_FC(&accumulation_buffers[i][0], 
                &phase_centers[0][next_sub_integration-1][0],
                n_fft);
    // UV shift the additional phase centers
    for(int j = 1; j < n_phase_centers; j++){
      double delay1 = delay_tables[station1].delay(tmid);
      double delay2 = delay_tables[station2].delay(tmid);
      double ddelay1 = delay_tables[station1].delay(tmid, j)-delay1;
      double ddelay2 = delay_tables[station2].delay(tmid, j)-delay2;
      double rate1 = delay_tables[station1].rate(tmid);
      double rate2 = delay_tables[station2].rate(tmid);
      uvshift(accumulation_buffers[i], phase_centers[j][next_sub_integration-1], 
              ddelay1, ddelay2, rate1, rate2);
    }
  }
  // Clear the accumulation buffers
  for (size_t i=0; i<accumulation_buffers.size(); i++) {
    SFXC_ASSERT(accumulation_buffers[i].size() == n_fft);
    size_t size = accumulation_buffers[i].size() * sizeof(std::complex<FLOAT>);
    memset(&accumulation_buffers[i][0], 0, size);
  }
  previous_fft = current_fft;
}

