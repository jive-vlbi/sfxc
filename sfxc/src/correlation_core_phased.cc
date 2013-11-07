#include "correlation_core_phased.h"
#include "output_header.h"
#include <utils.h>

Correlation_core_phased::Correlation_core_phased()
{
 is_open_ = false;
}

Correlation_core_phased::~Correlation_core_phased()
{
}

void
Correlation_core_phased::do_task() {
  SFXC_ASSERT(has_work());

  if (current_fft % 1000 == 0) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);
  }

  if (current_fft % number_ffts_in_integration == 0) {
    integration_initialise();
  }

  SFXC_ASSERT(input_buffers.size()==number_input_streams_in_use());
  for (size_t i = 0; i < number_input_streams_in_use(); i++) {
    int stream = streams_in_scan[i];
    input_elements[i] = &input_buffers[stream]->front()->data[0];
  }
  const Time tfft = correlation_parameters.integration_time / number_ffts_in_integration;
  Time time = correlation_parameters.start_time + tfft * 0.5 + tfft * current_fft;
  int polarisation = correlation_parameters.polarisation == 'R'? 0 : 1;
  double freq = correlation_parameters.channel_freq;
  //std::cout.precision(16);
  //if (RANK_OF_NODE == 10) std::cout << "freq = " << freq << "\n"; 
  const int first_stream = streams_in_scan[0];
  const int stride = input_buffers[first_stream]->front()->stride;
  const int nbuffer = input_buffers[first_stream]->front()->data.size() / stride;
  for (int buf = 0; buf < nbuffer * stride ; buf += stride){
    #ifndef DUMMY_CORRELATION
    int sub_integration = current_fft / number_ffts_in_sub_integration;
    for (size_t i = 0; i < number_input_streams_in_use(); i++) {
      size_t stream = 0;
      while (correlation_parameters.station_streams[stream].station_stream != streams_in_scan[i])
        stream++; 
      size_t station = correlation_parameters.station_streams[stream].station_number;
      btable.apply_bandpass(time, &input_elements[i][buf], station, freq, correlation_parameters.sideband, polarisation);
      cltable.apply_callibration(time, &input_elements[i][buf], station, freq, correlation_parameters.sideband, polarisation);
      if(i==0){
        memcpy(&subint_buffer[0], &input_elements[0][buf], (fft_size() + 1) * sizeof(std::complex<FLOAT>));
        memset(&autocor_buffer[0], 0, (fft_size() + 1) * sizeof(std::complex<FLOAT>));
      }
      else
        SFXC_ADD_FC(&input_elements[i][buf], &subint_buffer[0], fft_size() + 1);
      // Also accumulate the auto correlations
      SFXC_CONJ_FC(&input_elements[i][buf], &autocor_conj_buffer[0], fft_size() + 1);
      SFXC_ADD_PRODUCT_FC(&input_elements[i][buf], &autocor_conj_buffer[0], &autocor_buffer[0], fft_size() + 1);
      //if(RANK_OF_NODE == 10) std::cout <<"idx = "<< i << "station = " << station << "time = " << time << "\n";
    }
    SFXC_CONJ_FC(&subint_buffer[0], &subint_conj_buffer[0], fft_size() + 1);
    SFXC_MULC(&subint_buffer[0], &subint_conj_buffer[0], &power_buffer[0], fft_size() + 1);
    SFXC_SUB_FC(&autocor_buffer[0], &power_buffer[0], fft_size() + 1);
    SFXC_ADD_FC(&power_buffer[0], &accumulation_buffers[sub_integration][0], fft_size() + 1);
    current_fft++;
    time += tfft;
    //if(current_fft % number_ffts_in_sub_integration == 0)
    //  std::cout << RANK_OF_NODE << " : SUBINT " << sub_integration << " / " << number_ffts_in_sub_integration << "\n";
    #else // DUMMY_CORRELATION
    current_fft++;
    #endif // DUMMY_CORRELATION
  }
  for (size_t i = 0; i < number_input_streams_in_use(); i++){
    int stream = streams_in_scan[i];
    input_buffers[stream]->pop();
  }

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    integration_normalize();
    integration_write_headers(0, 0);
    integration_write_subints();
    current_integration++;
  }
}

void
Correlation_core_phased::set_parameters(const Correlation_parameters &parameters,
					int node_nr)
{
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;

  correlation_parameters = parameters;
  oversamp = (int) round(parameters.sample_rate / (2 * parameters.bandwidth));

  create_baselines(parameters);
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  subint_conj_buffer.resize(fft_size()+1);
  autocor_conj_buffer.resize(fft_size()+1);
  n_flagged.resize(baselines.size());
  try{
    if(!is_open_){
      btable.open_table("/home/keimpema/data/gv020g/gv020g.m15a.bp", fft_size(), true);
      //btable.open_table("/home/keimpema/data/fp003/fp003.bp", fft_size(), true);
      //cltable.open_table("/home/keimpema/data/fp003/fp003.cl", fft_size());
      cltable.open_table("/home/keimpema/data/gv020g/gv020g.m15a.cl", fft_size());
      is_open_ = true;
    }
  }catch(const std::string s){
   std::cerr << "Problem : " << s << "\n";
   throw s;
  }
  get_input_streams();
}

void
Correlation_core_phased::create_baselines(const Correlation_parameters &parameters){
  number_ffts_in_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size_correlation); 
  // One less because of the overlapping windows
  if (parameters.window != SFXC_WINDOW_NONE)
    number_ffts_in_integration -= 1;

  number_ffts_in_sub_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.sub_integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size_correlation);
  if(RANK_OF_NODE == 9) 
    std::cerr << "number_ffts_in_sub_integration = " << number_ffts_in_sub_integration 
              << ", number_ffts_in_integration = " << number_ffts_in_integration << "\n";
  baselines.clear();
  baselines.resize((number_ffts_in_integration+1) / number_ffts_in_sub_integration);
//  if (baselines.size() != 2000)
//    std::cerr << "Baseline.size = " << baselines.size()
//              << ", number_ffts_in_sub_integration = " << number_ffts_in_sub_integration 
//              << ", number_ffts_in_integration = " << number_ffts_in_integration << "\n";
}

void Correlation_core_phased::integration_initialise() {
  int num_sub_integrations =
    ((number_ffts_in_integration+1) / number_ffts_in_sub_integration);
  if (accumulation_buffers.size() != num_sub_integrations) {
    accumulation_buffers.resize(num_sub_integrations);
    for (size_t i = 0; i < accumulation_buffers.size(); i++) {
      accumulation_buffers[i].resize(fft_size() + 1);
    }
  }

  for (size_t i = 0; i < accumulation_buffers.size(); i++) {
    size_t size = accumulation_buffers[i].size() * sizeof(std::complex<FLOAT>);
    memset(&accumulation_buffers[i][0], 0, size);
  }
  subint_buffer.resize(fft_size() + 1);
  autocor_buffer.resize(fft_size() + 1);
  power_buffer.resize(fft_size() + 1);
  memset(&n_flagged[0], 0, sizeof(std::pair<int64_t,int64_t>)*n_flagged.size());
}

void Correlation_core_phased::integration_normalize() {

}

void Correlation_core_phased::integration_write_subints() {
  
  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());

  int polarisation = (correlation_parameters.polarisation == 'R')? 0 : 1;

  std::vector<float> integration_buffer;
  integration_buffer.resize(number_channels() + 1);
  Output_header_baseline hbaseline;

  size_t n = fft_size() / number_channels();
  for (size_t i = 0; i < baselines.size(); i++) {
    for (size_t j = 0; j < number_channels(); j++) {
      integration_buffer[j] = accumulation_buffers[i][j * n].real();
      for (size_t k = 1; k < n ; k++)
        integration_buffer[j] += accumulation_buffers[i][j * n + k].real();
      integration_buffer[j] /= n;
    }
    integration_buffer[number_channels()] = accumulation_buffers[i][number_channels() * n].real();
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
                      ((char*)&integration_buffer[0]));
  }
}

