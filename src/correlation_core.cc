#include "correlation_core.h"

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
    
    if (current_fft % 1000 == 0) {
      DEBUG_MSG(current_fft << " of " << number_ffts_in_integration);
    }
    
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
  accumulation_buffers.resize((size_of_fft()/2+1) * baselines.size());
  
  for (size_t i=0; i<accumulation_buffers.size(); i++) {
    accumulation_buffers[i] = 0;
  }
  
  current_fft = 0;
}

void Correlation_core::integration_step() {
  std::vector<Input_buffer_element *> input_elements;
  input_elements.resize(input_buffers.size());
  for (size_t i=0; i<input_buffers.size(); i++) {
    int size;
    input_elements[i] = &input_buffers[i]->consume(size);
    assert(size == size_of_fft());
    assert(size == input_elements[i]->size());
  }
  
  // Do the fft from time to frequency:
  FFTW_PLAN           plan;
  assert(frequency_buffer.size() == 
         correlation_parameters.station_streams.size());
  for (size_t i=0; i<input_buffers.size(); i++) {
    assert(frequency_buffer[i].size() == size_of_fft()/2+1);
    plan = FFTW_PLAN_DFT_R2C_1D(size_of_fft(), 
                                (DOUBLE *)input_elements[i]->buffer(),
                                (FFTW_COMPLEX *)&frequency_buffer[i][0],
                                FFTW_ESTIMATE);
    FFTW_EXECUTE(plan);
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
  std::vector<DOUBLE> norms;
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
    DOUBLE norm = sqrt(norms[stations.first]*norms[stations.second]);
    for (size_t i = 0 ; i < size_of_fft()/2+1; i++){
      accumulation_buffers[station*(size_of_fft()/2+1)+i] /= norm;
    }
  }
}

void Correlation_core::integration_write() {
  assert(writer != boost::shared_ptr<Data_writer>());
  writer->put_bytes(accumulation_buffers.size()*sizeof(std::complex<DOUBLE>),
                    ((char*)&accumulation_buffers[0]));
}

void 
Correlation_core::
auto_correlate_baseline(std::complex<DOUBLE> in[],
                        std::complex<DOUBLE> out[]) {
  for (size_t i=0; i<size_of_fft()/2+1; i++) {
    out[i].real() += in[i].real()*in[i].real() +
                     in[i].imag()*in[i].imag();
    out[i].imag() = 0;
  }
}

void 
Correlation_core::
correlate_baseline(std::complex<DOUBLE> in1[],
                   std::complex<DOUBLE> in2[],
                   std::complex<DOUBLE> out[]) {
  // NGHK: TODO: expand and optimize
  for (size_t i=0; i<size_of_fft()/2+1; i++) {
    //out[i] += in1[i]*std::conj(in2[i]);
    out[i].real() += in1[i].real() * in2[i].real() + in1[i].imag() * in2[i].imag();
    out[i].imag() += in1[i].imag() * in2[i].real() - in1[i].real() * in2[i].imag();
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

