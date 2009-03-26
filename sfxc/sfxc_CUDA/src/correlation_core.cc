#include "correlation_core.h"
#include "output_header.h"
#include <utils.h>

Correlation_core::Correlation_core(int swap_)
    : current_fft(0), total_ffts(0), swap(swap_), check_input_elements(true){
  #ifdef SFXC_WRITE_STATS
    SFXC_ASSERT_MSG(swap==0,
                    "SFXC_WRITE_STATS currently doesn't work if swap is set");
  #endif
}

Correlation_core::~Correlation_core() {
#if PRINT_TIMER
  int N = size_of_fft();
  int numiterations = total_ffts;
  double time = fft_timer.measured_time()*1000000;
  PROGRESS_MSG("MFlops: " << 5.0*N*log2(N) * numiterations / (1.0*time));
#endif
}

void Correlation_core::do_task() {
  SFXC_ASSERT(has_work());
  if (current_fft % 1000 == 0) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);
  }

  if (current_fft%number_ffts_in_integration == 0) {
    integration_initialise();
  }

  // Process the data of the current fft
  integration_step();
  current_fft ++;

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    integration_average();
    integration_write();
  }
}

bool Correlation_core::almost_finished() {
  return current_fft == number_ffts_in_integration*9/10;
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
Correlation_core::set_parameters(const Correlation_parameters &parameters,
                                 int node_nr) {
  node_nr_ = node_nr;
  current_integration = 0;
  current_fft = 0;

  size_t prev_size_of_fft = size_of_fft();
  correlation_parameters = parameters;

  number_ffts_in_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);

  baselines.clear();
  // Autos
  for (size_t sn = 0 ; sn < n_stations(); sn++) {
    baselines.push_back(std::pair<int,int>(sn,sn));
  }
  // Crosses
  int ref_station = parameters.reference_station;
  if (parameters.cross_polarize) {
    SFXC_ASSERT(n_stations() % 2 == 0);
    size_t n_st_2 = n_stations()/2;
    if (ref_station >= 0) {
      // cross polarize with a reference station
      for (int sn = 0 ; sn < ref_station; sn++) {
        baselines.push_back(std::make_pair(sn       , ref_station       ));
        baselines.push_back(std::make_pair(sn+n_st_2, ref_station       ));
        baselines.push_back(std::make_pair(sn       , ref_station+n_st_2));
        baselines.push_back(std::make_pair(sn+n_st_2, ref_station+n_st_2));
      }
      for (size_t sn = ref_station+1 ; sn < n_st_2; sn++) {
        baselines.push_back(std::make_pair(ref_station       , sn       ));
        baselines.push_back(std::make_pair(ref_station       , sn+n_st_2));
        baselines.push_back(std::make_pair(ref_station+n_st_2, sn       ));
        baselines.push_back(std::make_pair(ref_station+n_st_2, sn+n_st_2));
      }
    } else {
      // cross polarize without a reference station
      for (size_t sn = 0 ; sn < n_st_2 - 1; sn++) {
        for (size_t sno = sn + 1; sno < n_st_2; sno ++) {
          baselines.push_back(std::make_pair(sn       ,sno));
          baselines.push_back(std::make_pair(sn       ,sno+n_st_2));
          baselines.push_back(std::make_pair(sn+n_st_2,sno));
          baselines.push_back(std::make_pair(sn+n_st_2,sno+n_st_2));
        }
      }
    }
  } else { // No cross_polarisation
    if (parameters.reference_station >= 0) {
      // no cross polarisation with a reference station
      for (int sn = 0 ; sn < (int)n_stations(); sn++) {
        if (sn != ref_station) {
          baselines.push_back(std::pair<int,int>(sn,ref_station));
        }
      }
    } else { // No reference station
      // no cross polarisation without a reference station

      for (size_t sn = 0 ; sn < n_stations() - 1; sn++) {
        for (size_t sno = sn + 1; sno < n_stations() ; sno ++) {
          baselines.push_back(std::pair<int,int>(sn,sno));
        }
      }
    }
  }

#ifdef SFXC_WRITE_STATS
  backward_buffer.resize(size_of_fft()/2+1);
  backward_plan_ =
  FFTW_PLAN_DFT_1D(size_of_fft()/2+1,
                   reinterpret_cast<FFTW_COMPLEX*>(plan_output_buffer.buffer()),
                   reinterpret_cast<FFTW_COMPLEX*>(&backward_buffer[0]),
                   FFTW_BACKWARD,
                   FFTW_ESTIMATE);
#endif // SFXC_WRITE_STATS
}

void
Correlation_core::
set_data_writer(boost::shared_ptr<Data_writer> writer_) {
  writer = writer_;
}

bool Correlation_core::has_work() {
  if(check_input_elements){
    for (size_t i=0, nstreams=number_input_streams_in_use(); i < nstreams; i++) {
      if (input_buffers[i]->empty())
        return false;
    }
  }
  return true;
}

void Correlation_core::integration_initialise() {
  if (accumulation_buffers.size() != baselines.size()) {
    accumulation_buffers.resize(baselines.size());
    size_t size = (size_of_fft()/2+1);
    for (size_t i=0; i<accumulation_buffers.size(); i++) {
      accumulation_buffers[i].resize(size);
    }
  }

  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());
  for (size_t i=0; i<accumulation_buffers.size(); i++) {
    SFXC_ASSERT(accumulation_buffers[i].size() == size_of_fft()/2+1);
    for (size_t j=0; j<accumulation_buffers[i].size(); j++) {
      accumulation_buffers[i][j] = 0;
    }
  }
}

void Correlation_core::integration_step() {
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  if(check_input_elements){
    for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++) {
      if(!input_elements[i].valid()) 
        input_elements[i].set(&input_buffers[i]->front().data()[0],
                              input_buffers[i]->front().data().size());
    }
    check_input_elements=false;
  }

#ifndef DUMMY_CORRELATION
  // do the correlation
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    // Auto correlations
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first == stations.second);
    auto_correlate_baseline(/* in1 */ &input_elements[stations.first][0],
                            /* out */ &accumulation_buffers[i][0]);
  }

  for (size_t i=number_input_streams_in_use(); i < baselines.size(); i++) {
    // Cross correlations
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first != stations.second);
    correlate_baseline(/* in1 */  &input_elements[stations.first][0],
                       /* in2 */  &input_elements[stations.second][0],
                       /* out */ &accumulation_buffers[i][0]);
  }

#ifdef SFXC_WRITE_STATS
  {
#ifndef SFXC_DETERMINISTIC
    SFXC_ASSERT_MSG(false,
                    "SFXC_WRITE_STATS only works with SFXC_DETERMINISTIC");
#endif

    if (! stats_out.is_open()) {
      char filename[80];
      snprintf(filename, 80, "stats_%d.txt", RANK_OF_NODE);
      stats_out.open(filename);
    }
    SFXC_ASSERT(stats_out.is_open());

    // Reset buffer:
    for (size_t i=0; i<size_of_fft()/2+1; i++)
      backward_buffer[i] = 0;

    int baseline = number_input_streams_in_use();
    std::pair<size_t,size_t> &stations = baselines[baseline];

    correlate_baseline
      (/* in1 */ input_elements[stations.first].buffer(),
       /* in2 */ input_elements[stations.second].buffer(),
       /* out */ &backward_buffer[0]);

    // Hardcode the position of the fringe here
    const int fringe_pos = 12;

    FFTW_EXECUTE_DFT(backward_plan_,
                     (FFTW_COMPLEX *)&backward_buffer[0],
                     (FFTW_COMPLEX *)&backward_buffer[0]);
    FLOAT fft_abs   = std::abs(backward_buffer[fringe_pos]);
    FLOAT fft_phase = std::arg(backward_buffer[fringe_pos]);

    FFTW_EXECUTE_DFT(backward_plan_,
                     (FFTW_COMPLEX *)&accumulation_buffers[baseline][0],
                     (FFTW_COMPLEX *)&backward_buffer[0]);
    FLOAT integr_abs   = std::abs(backward_buffer[fringe_pos]);
    FLOAT integr_phase = std::arg(backward_buffer[fringe_pos]);
    int max_pos = 0;
    for (size_t i=1; i<size_of_fft()/2+1; i++) {
      if (std::abs(backward_buffer[i]) > std::abs(backward_buffer[max_pos]))
        max_pos = i;
    }

    stats_out << fft_abs << " \t" << fft_phase << " \t"
              << integr_abs << " \t" << integr_phase << " \t"
              << max_pos << std::endl;
  }
#endif // SFXC_WRITE_STATS
#endif // DUMMY_CORRELATION


  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++){
    input_elements[i]++; // advance the iterator
    if(!input_elements[i].valid()){
      input_buffers[i]->pop();
      check_input_elements=true;
    }
  }
}

void Correlation_core::integration_average() {
  std::vector<FLOAT> norms;
  norms.resize(n_stations());
  for (size_t i=0; i<norms.size(); i++) norms[i] = 0;

  // Average the auto correlations
  for (size_t station=0; station < n_stations(); station++) {
    for (size_t i = 0; i < size_of_fft()/2+1; i++) {
      norms[station] += accumulation_buffers[station][i].real();
    }
    norms[station] /= (size_of_fft()/2);

    for (size_t i = 0; i < size_of_fft()/2+1; i++) {
      // imaginary part should be zero!
      accumulation_buffers[station][i] =
        accumulation_buffers[station][i].real() / norms[station];
    }
  }

  // Average the cross correlations
  for (size_t station=n_stations(); station < baselines.size(); station++) {
    std::pair<size_t,size_t> &stations = baselines[station];
    FLOAT norm = sqrt(norms[stations.first]*norms[stations.second]);
    for (size_t i = 0 ; i < size_of_fft()/2+1; i++) {
      accumulation_buffers[station][i] /= norm;
    }
  }
}

void Correlation_core::integration_write() {

  // Make sure that the input buffers are released
  // This is done by reference counting

  SFXC_ASSERT(writer != boost::shared_ptr<Data_writer>());
  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());

  int polarisation = 1;
  if (correlation_parameters.polarisation == 'R') {
    polarisation =0;
  } else {
    SFXC_ASSERT(correlation_parameters.polarisation == 'L');
  }

  { // Writing the timeslice header
    Output_header_timeslice htimeslice;

    htimeslice.number_baselines = baselines.size();
    htimeslice.integration_slice =
      correlation_parameters.integration_nr + current_integration;
    htimeslice.number_uvw_coordinates = uvw_tables.size();

   // write the uvw coordinates
    Output_uvw_coordinates uvw[htimeslice.number_uvw_coordinates];
    // We evaluate in the middle of time slice (nb: the factor 1000 is for the conversion to microseconds)
    int64_t time;
    time=(int64_t)correlation_parameters.start_time*1000+(int64_t)correlation_parameters.integration_time*500;
    for (size_t station=0; station < uvw_tables.size(); station++){
     double u,v,w;
     uvw_tables[station].get_uvw(time, &u, &v, &w);
     uvw[station].station_nr=station+1;
     uvw[station].u=u;
     uvw[station].v=v;
     uvw[station].w=w;
    }

    size_t nWrite = sizeof(htimeslice);
    writer->put_bytes(nWrite, (char *)&htimeslice);
    nWrite=sizeof(uvw);
    writer->put_bytes(nWrite, (char *)&uvw[0]);

    current_integration++;
  }

  accumulation_buffers_float.resize(size_of_fft()/2+1);
  size_t n_stations = correlation_parameters.station_streams.size();
  std::vector<int> stream2station;

  {
    // initialise with -1
    stream2station.resize(input_buffers.size(), -1);

    for (size_t i=0;
         i < correlation_parameters.station_streams.size();
         i++) {
      size_t station_stream =
        correlation_parameters.station_streams[i].station_stream;
      SFXC_ASSERT(station_stream < stream2station.size());
      stream2station[station_stream] =
        correlation_parameters.station_streams[i].station_number;
    }
  }

  Output_header_baseline hbaseline;
  for (size_t i=0; i<baselines.size(); i++) {
    std::pair<size_t,size_t> &stations = baselines[i];

    for (size_t ii=0; ii<(size_of_fft()/2+1); ii++ ) {
      accumulation_buffers_float[ii] = accumulation_buffers[i][ii];
    }

    hbaseline.weight = 0;       // The number of good samples

    // Station number in the vex-file
    SFXC_ASSERT(stations.first < n_stations);
    SFXC_ASSERT(stations.second < n_stations);
    hbaseline.station_nr1 = stream2station[stations.first];
    // Station number in the vex-file
    hbaseline.station_nr2 = stream2station[stations.second];

    // Polarisation for the first station
    SFXC_ASSERT((polarisation == 0) || (polarisation == 1)); // (RCP: 0, LCP: 1)
    hbaseline.polarisation1 = polarisation;
    hbaseline.polarisation2 = polarisation;
    if (correlation_parameters.cross_polarize) {
      if (stations.first >= n_stations/2)
        hbaseline.polarisation1 = 1-polarisation;
      if (stations.second >= n_stations/2)
        hbaseline.polarisation2 = 1-polarisation;
    }
    // Upper or lower sideband (LSB: 0, USB: 1)
    if (correlation_parameters.sideband=='U') {
      hbaseline.sideband = 1;
    } else {
      SFXC_ASSERT(correlation_parameters.sideband == 'L');
      hbaseline.sideband = 0;
    }
    // The number of the channel in the vex-file,
    hbaseline.frequency_nr = (unsigned char)correlation_parameters.channel_nr;
    // sorted increasingly
    // 1 byte left:
    hbaseline.empty = ' ';

    int nWrite = sizeof(hbaseline);
    writer->put_bytes(nWrite, (char *)&hbaseline);
    writer->put_bytes((size_of_fft()/2+1)*sizeof(std::complex<float>),
                      ((char*)&accumulation_buffers_float[0]));
  }
}

void
Correlation_core::
auto_correlate_baseline(std::complex<FLOAT> in[],
                        std::complex<FLOAT> out[]) {
  int size = size_of_fft()/2+1;
  for (int i=0; i<size; i++) {
    out[i] += (in[i].real()*in[i].real() +
               in[i].imag()*in[i].imag());
  }
}

void
Correlation_core::
correlate_baseline(std::complex<FLOAT> in1[],
                   std::complex<FLOAT> in2[],
                   std::complex<FLOAT> out[]) {
  int size = size_of_fft()/2+1;
  for (int i=0; i<size; i++) {
    out[i] += in1[i]*std::conj(in2[i]);
  }
}

void Correlation_core::add_uvw_table(int sn, Uvw_model &table) {
  if (sn>=uvw_tables.size())
    uvw_tables.resize(sn+1);

  uvw_tables[sn]=table;
}
