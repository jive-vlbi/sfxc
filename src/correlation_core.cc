#include "correlation_core.h"

Correlation_core::Correlation_core()
: output_buffer(Output_buffer_ptr(new Output_buffer(2))),
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
  if (has_work()) {
    if (current_fft%number_ffts_in_integration == 0) {
      integration_initialise();
    }

#if 1
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
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
//  number_ffts_in_integration = 
//    (int)std::floor(parameters.integration_time/1000. * 
//                    parameters.sample_rate * 1./parameters.number_channels);
  
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
      for (int sn = 0 ; sn < (int)n_stations(); sn++){
        if ((sn != ref_station) && (sn != (ref_station+(int)n_stations()/2))) {
          baselines.push_back(std::pair<int,int>(sn,ref_station));
        }
      }
      for (int sn = 0 ; sn < (int)n_stations(); sn++){
        if ((sn != ref_station) && (sn != (ref_station+(int)n_stations()/2))) {
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
      for (int sn = 0 ; sn < (int)n_stations(); sn++){
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

bool Correlation_core::has_work() {
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
  size_t size = (size_of_fft()/2+1) * baselines.size();
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
    assert(size == (int)size_of_fft());
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
  writer->put_bytes(accumulation_buffers.size()*sizeof(std::complex<FLOAT>),
                    ((char*)&accumulation_buffers[0]));
}

void 
Correlation_core::
auto_correlate_baseline(std::complex<FLOAT> in[],
                        std::complex<FLOAT> out[]) {
  int size = size_of_fft()/2+1;
  for (int i=0; i<size; i++) {
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
  for (int i=0; i<size; i++) {
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

