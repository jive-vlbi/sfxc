#include "correlation_core.h"
#include "output_header.h"
#include <utils.h>

Correlation_core::Correlation_core()
: output_buffer(Output_buffer_ptr(new Output_buffer(10))),
  current_fft(0)
{
}

Correlation_core::~Correlation_core()
{
  DEBUG_MSG(timer);
}

Correlation_core::Output_buffer_ptr 
Correlation_core::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void Correlation_core::do_task() {
  timer.resume();
  if (is_ready_for_do_task()) {
    if (current_fft%number_ffts_in_integration == 0) {
      integration_initialise();
    }

#if 0
    if (current_fft % 1000 == 0) {
      DEBUG_MSG(current_fft << " of " << number_ffts_in_integration);
    }
#endif
    
    // Process the data of the current fft
    integration_step();
    current_fft ++;
    
    if (current_fft == number_ffts_in_integration) {
      integration_average();
      integration_write();
    }
  }
  timer.stop();
}

bool Correlation_core::finished() {
  return current_fft == number_ffts_in_integration;
}

void Correlation_core::connect_to(size_t stream, Input_buffer_ptr buffer) {
  if (stream >= input_buffers.size()) {
    input_buffers.resize(stream+1);
  }
  input_buffers[stream] = buffer;
}


void 
Correlation_core::set_parameters(const Correlation_parameters &parameters) {
  correlation_parameters = parameters;
  
  number_ffts_in_integration = 
    (int)std::floor(parameters.integration_time/1000. * 
                    parameters.sample_rate * 1./parameters.number_channels);
  
  baselines.clear();
  // Autos
  for (size_t sn = 0 ; sn < n_stations(); sn++){
    baselines.push_back(std::pair<int,int>(sn,sn));
  }
  // Crosses
  int ref_station = parameters.reference_station;
  if (parameters.cross_polarize) {
    assert(n_stations() % 2 == 0);
    if (ref_station >= 0) {
      // cross polarize with a reference station
      for (size_t sn = 0 ; sn < n_stations(); sn++){
        if ((sn != ref_station) && (sn != (ref_station+n_stations()/2))) {
          baselines.push_back(std::pair<int,int>(sn,ref_station));
        }
      }
      for (size_t sn = 0 ; sn < n_stations(); sn++){
        if ((sn != ref_station) && (sn != (ref_station+n_stations()/2))) {
          baselines.push_back(std::pair<int,int>(sn,ref_station+n_stations()/2));
        }
      }
    } else {
      // cross polarize without a reference station
      for (size_t sn = 0 ; sn < n_stations() - 1; sn++){
        for (size_t sno = sn + 1; sno < n_stations() ; sno ++){
          if (sn+n_stations()/2 != sno) {
            // Do not cross correlate the 
            // two polarisations of the same station
            baselines.push_back(std::pair<int,int>(sn,sno));
          }
        }
      }
    }
  } else { // No cross_polarisation
    if (parameters.reference_station >= 0) {
      // no cross polarisation with a reference station
      for (size_t sn = 0 ; sn < n_stations(); sn++){
        if (sn != ref_station) {
          baselines.push_back(std::pair<int,int>(sn,ref_station));
        }
      }
    } else { // No reference station
      // no cross polarisation without a reference station

      for (size_t sn = 0 ; sn < n_stations() - 1; sn++){
        for (size_t sno = sn + 1; sno < n_stations() ; sno ++){
          baselines.push_back(std::pair<int,int>(sn,sno));
        }
      }
    }
  }
  
  frequency_buffer.resize(correlation_parameters.station_streams.size());
  for (size_t i=0; i < frequency_buffer.size(); i++) {
    frequency_buffer[i].resize(size_of_fft()/2+1);
  }

  plan_input_buffer.resize(size_of_fft());
  plan_output_buffer.resize(size_of_fft()/2+1);
  plan = FFTW_PLAN_DFT_R2C_1D(size_of_fft(), 
                              (FLOAT *)&plan_input_buffer[0],
                              (FFTW_COMPLEX *)&plan_output_buffer[0],
                              FFTW_MEASURE);
}

void 
Correlation_core::
set_data_writer(boost::shared_ptr<Data_writer> writer_) {
  writer = writer_;
}

bool Correlation_core::is_ready_for_do_task() {
  for (size_t i=0; i < input_buffers.size(); i++) {
    if (input_buffers[i]->empty()) {
      return false;
    }
  }
  if ((current_fft == 0) && output_buffer->full()) {
    return false;
  }
  return true;
}

void Correlation_core::integration_initialise() {
  int size = (size_of_fft()/2+1) * baselines.size();
  if (accumulation_buffers.size() != size) {
    accumulation_buffers.resize(size);
  }
  
  for (size_t i=0; i<accumulation_buffers.size(); i++) {
    accumulation_buffers[i] = 0;
  }
  
  current_fft = 0;
}

void Correlation_core::integration_step() {
  if (input_elements.size() != input_buffers.size()) {
    input_elements.resize(input_buffers.size());
  }
  for (size_t i=0; i<input_buffers.size(); i++) {
    int size;
    input_elements[i] = &input_buffers[i]->consume(size);
    assert(size == size_of_fft());
    assert(size == input_elements[i]->size());
  }
  
  // Do the fft from time to frequency:
  assert(frequency_buffer.size() == 
         correlation_parameters.station_streams.size());
  for (size_t i=0; i<input_buffers.size(); i++) {
    assert(frequency_buffer[i].size() == size_of_fft()/2+1);
    FFTW_EXECUTE_DFT_R2C(plan, 
                         (FLOAT *)input_elements[i]->buffer(),
                         (FFTW_COMPLEX *)&frequency_buffer[i][0]);
  }

  // do the correlation
  for (size_t i=0; i < input_buffers.size(); i++) {
    // Auto correlations
    std::pair<int,int> &stations = baselines[i];
    assert(stations.first == stations.second);
    auto_correlate_baseline(/* in1 */ 
                            &frequency_buffer[stations.first][0],
                            /* out */ 
                            &accumulation_buffers[i*(size_of_fft()/2+1)]);
  }
  
  for (size_t i=input_buffers.size(); i < baselines.size(); i++) {
    // Cross correlations
    std::pair<int,int> &stations = baselines[i];
    assert(stations.first != stations.second);
    correlate_baseline
      (/* in1 */ &frequency_buffer[stations.first][0], 
       /* in2 */ &frequency_buffer[stations.second][0],
       /* out */ &accumulation_buffers[i*(size_of_fft()/2+1)]);
  }

  for (size_t i=0; i<input_buffers.size(); i++) {
    input_buffers[i]->consumed();
  }
}

void Correlation_core::integration_average() {
  std::vector<FLOAT> norms;
  norms.resize(n_stations(), 0);
  
  // Average the auto correlations
  for (size_t station=0; station < n_stations(); station++) {
    for (size_t i = 0; i < size_of_fft()/2+1; i++) {
      norms[station] += accumulation_buffers[station*(size_of_fft()/2+1)+i].real();
    }
    for (size_t i = 0; i < size_of_fft()/2+1; i++) {
      // imaginary part should be zero!
      accumulation_buffers[station*(size_of_fft()/2+1)+i].real() /= norms[station];
    }
  }

  // Average the cross correlations
  for (size_t station=n_stations(); station < baselines.size(); station++) {
    std::pair<int,int> &stations = baselines[station];
    FLOAT norm = sqrt(norms[stations.first]*norms[stations.second]);
    for (size_t i = 0 ; i < size_of_fft()/2+1; i++){
      accumulation_buffers[station*(size_of_fft()/2+1)+i] /= norm;
    }
  }
}

void Correlation_core::integration_write() {
  assert(writer != boost::shared_ptr<Data_writer>());
  assert(accumulation_buffers.size() ==
    baselines.size()*(size_of_fft()/2+1));

  int nr_corr = (correlation_parameters.stop_time-correlation_parameters.start_time)
                /correlation_parameters.integration_time;

  int polarisation;
  if(correlation_parameters.polarisation == 'R'){
    polarisation =0;
  } else if (correlation_parameters.polarisation == 'L'){
    polarisation =1;
  }

  Output_header_timeslice htimeslice;
  Output_header_baseline hbaseline;
  
  htimeslice.number_baselines = baselines.size();
  htimeslice.integration_slice = nr_corr;
  htimeslice.number_uvw_coordinates = 0;

  //write normalized correlation results to output file
  //NGHK: Make arrays consecutive to be able to write all data at once
  
  uint64_t nWrite = sizeof(htimeslice);
  writer->put_bytes(nWrite, (char *)&htimeslice);
  
  std::vector<int32_t> station_list;
  for (int ii=0; ii<correlation_parameters.station_number.size(); ii++){
       station_list.push_back(correlation_parameters.station_number[ii]);
  }
  
  accumulation_buffers_float.resize(size_of_fft()/2+1);
  
  for (int i=0; i<baselines.size(); i++) {
    std::pair<int,int> &stations = baselines[i];
    
    for (int ii=0; ii<size_of_fft()/2+1; ii++ ){
      accumulation_buffers_float[ii] = accumulation_buffers[i*(size_of_fft()/2+1)+ii];
    }

    hbaseline.weight = 0;       // The number of good samples
    hbaseline.station_nr1 = (uint8_t)station_list[stations.first];  // Station number in the vex-file
    hbaseline.station_nr2 = (uint8_t)station_list[stations.second];  // Station number in the vex-file
    hbaseline.polarisation1 = (unsigned char)polarisation; // Polarisation for the first station
                            // (RCP: 0, LCP: 1)
    hbaseline.polarisation2 = (unsigned char)polarisation; // Polarisation for the second station
    if(correlation_parameters.sideband=='U'){
      hbaseline.sideband = 1;      // Upper or lower sideband (LSB: 0, USB: 1)
    }else if(correlation_parameters.sideband=='L'){
      hbaseline.sideband = 0;
    }
    hbaseline.frequency_nr = (unsigned char)correlation_parameters.channel_nr;       // The number of the channel in the vex-file,
                            // sorted increasingly
      // 1 byte left:
    hbaseline.empty = ' ';
    
    nWrite = sizeof(hbaseline);
    writer->put_bytes(nWrite, (char *)&hbaseline);
    writer->put_bytes((size_of_fft()/2+1)*sizeof(std::complex<float>),
        ((char*)&accumulation_buffers_float[0]));
  }
/*
  writer->put_bytes(accumulation_buffers.size()*sizeof(std::complex<DOUBLE>),
                    ((char*)&accumulation_buffers[0]));
*/
}

void 
Correlation_core::
auto_correlate_baseline(std::complex<FLOAT> in[],
                        std::complex<FLOAT> out[]) {
  int size = size_of_fft()/2+1;
  for (size_t i=0; i<size; i++) {
    out[i].real() += in[i].real()*in[i].real() +
                     in[i].imag()*in[i].imag();
    out[i].imag() = 0;
  }
}

void 
Correlation_core::
correlate_baseline(std::complex<FLOAT> in1[],
                   std::complex<FLOAT> in2[],
                   std::complex<FLOAT> out[]) {
  // NGHK: TODO: expand and optimize
  int size = size_of_fft()/2+1;
  for (size_t i=0; i<size; i++) {
    //out[i] += in1[i]*std::conj(in2[i]);
    out[i].real() += 
      in1[i].real() * in2[i].real() + 
      in1[i].imag() * in2[i].imag();
    out[i].imag() += 
      in1[i].imag() * in2[i].real() - 
      in1[i].real() * in2[i].imag();
  }
}

size_t Correlation_core::n_channels() {
  return correlation_parameters.number_channels;
}

size_t Correlation_core::size_of_fft() {
  return n_channels()*2;
}

size_t Correlation_core::n_stations() {
  return correlation_parameters.station_streams.size();
}

