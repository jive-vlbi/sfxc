#include "correlation_core.h"
#include "output_header.h"
#include <utils.h>

Correlation_core::Correlation_core()
    : current_fft(0), total_ffts(0), split_output(false){
}

Correlation_core::~Correlation_core() {
#if PRINT_TIMER
  int N = 2 * fft_size();
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

  SFXC_ASSERT(input_buffers.size()==number_input_streams_in_use());
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    input_elements[i] = &input_buffers[i]->front()->data[0];
  }
  find_invalid();
  const int stride = input_buffers[0]->front()->stride;
  const int nbuffer = input_buffers[0]->front()->data.size() / stride;
  for (int buf = 0; buf < nbuffer * stride ; buf += stride){
    // Process the data of the current fft
    integration_step(accumulation_buffers, buf);
    current_fft++;
  }
  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++)
    input_buffers[i]->pop();

  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    sub_integration();
    for(int i = 0 ; i < phase_centers.size(); i++){
      integration_normalize(phase_centers[i]);
      int source_nr;
      if(split_output){
        delay_tables[0].goto_scan(correlation_parameters.start_time);
        source_nr = sources[delay_tables[0].get_source(i)];
      }
      else
        source_nr = 0;
      integration_write(phase_centers[i], i, source_nr);
    }
    current_integration++;
  } else if(current_fft >= next_sub_integration * number_ffts_in_sub_integration){
    sub_integration();
    next_sub_integration++;
  }
}

bool Correlation_core::almost_finished() {
  return current_fft >= number_ffts_in_integration*9/10;
}

bool Correlation_core::finished() {
  return current_fft == number_ffts_in_integration;
}

void Correlation_core::connect_to(size_t stream, bit_statistics_ptr statistics_, Input_buffer_ptr buffer) {
  if (stream >= input_buffers.size()) {
    input_buffers.resize(stream+1);
    statistics.resize(stream+1);
  }
  input_buffers[stream] = buffer;
  statistics[stream] = statistics_;
}

void
Correlation_core::set_parameters(const Correlation_parameters &parameters,
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
  if (input_conj_buffers.size() != number_input_streams_in_use()) {
    input_conj_buffers.resize(number_input_streams_in_use());
    for(int i=0;i<number_input_streams_in_use();i++)
      input_conj_buffers[i].resize(fft_size() + 1);
  }
  n_flagged.resize(baselines.size());

#ifdef SFXC_WRITE_STATS
  backward_buffer.resize(fft_size() + 1);
  backward_plan_ =
    FFTW_PLAN_DFT_1D(fft_size() + 1,
                   reinterpret_cast<FFTW_COMPLEX*>(plan_output_buffer.buffer()),
                   reinterpret_cast<FFTW_COMPLEX*>(&backward_buffer[0]),
                   FFTW_BACKWARD,
                   FFTW_ESTIMATE);
#endif // SFXC_WRITE_STATS
}

void
Correlation_core::create_baselines(const Correlation_parameters &parameters){
  number_ffts_in_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size);

  number_ffts_in_sub_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.sub_integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size);
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
}

void
Correlation_core::
set_data_writer(boost::shared_ptr<Data_writer> writer_) {
  writer = writer_;
}

bool Correlation_core::has_work() {
  for (size_t i=0, nstreams=number_input_streams_in_use(); i < nstreams; i++) {
    if (input_buffers[i]->empty())
      return false;
  }
  return true;
}

void Correlation_core::integration_initialise() {
  previous_fft = 0;
  int start = accumulation_buffers.size() == baselines.size() ? phase_centers.size() : 0;

  if(phase_centers.size() != correlation_parameters.n_phase_centers)
    phase_centers.resize(correlation_parameters.n_phase_centers);

  for(int i = start ; i < phase_centers.size() ; i++){
    phase_centers[i].resize(baselines.size());
    for(int j = 0 ; j < phase_centers[i].size() ; j++){
      phase_centers[i][j].resize(fft_size() + 1);
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

void Correlation_core::integration_step(std::vector<Complex_buffer> &integration_buffer, int buf_idx) {
#ifndef DUMMY_CORRELATION
  // do the correlation
  for (size_t i=0; i < number_input_streams_in_use(); i++) {
    // get the complex conjugates of the input
    SFXC_CONJ_FC(&input_elements[i][buf_idx], &(input_conj_buffers[i])[0], fft_size() + 1);
    // Auto correlation
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first == stations.second);
    SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[stations.first][buf_idx], 
                        /* in2 */ &input_conj_buffers[stations.first][0],
                        /* out */ &integration_buffer[i][0], fft_size() + 1);
  }

  for (size_t i=number_input_streams_in_use(); i < baselines.size(); i++) {
    // Cross correlations
    std::pair<size_t,size_t> &stations = baselines[i];
    SFXC_ASSERT(stations.first != stations.second);
    SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[stations.first][buf_idx], 
                        /* in2 */ &input_conj_buffers[stations.second][0],
                        /* out */ &integration_buffer[i][0], fft_size() + 1);
  }

#ifdef SFXC_WRITE_STATS
  {
#ifndef SFXC_DETERMINISTIC
    sfxc_abort("SFXC_WRITE_STATS only works with SFXC_DETERMINISTIC\n");
#endif

    if (! stats_out.is_open()) {
      char filename[80];
      snprintf(filename, 80, "stats_%d.txt", RANK_OF_NODE);
      stats_out.open(filename);
    }
    SFXC_ASSERT(stats_out.is_open());

    // Reset buffer:
    for (size_t i = 0; i < fft_size() + 1; i++)
      backward_buffer[i] = 0;

    int baseline = number_input_streams_in_use();
    std::pair<size_t,size_t> &stations = baselines[baseline];

    SFXC_ADD_PRODUCT_FC
      (/* in1 */ &input_elements[stations.first][buf_idx],
       /* in2 */ &input_elements[stations.second][buf_idx],
       /* out */ &backward_buffer[0], fft_size() + 1);
      
    // Hardcode the position of the fringe here
    const int fringe_pos = 12;

    backward_fft.ifft(&backward_buffer[0], &backward_buffer[0]);
    FLOAT fft_abs   = std::abs(backward_buffer[fringe_pos]);
    FLOAT fft_phase = std::arg(backward_buffer[fringe_pos]);
      
    backward_fft.ifft(&integration_buffer[baseline][0], &backward_buffer[0]);
    FLOAT integr_abs   = std::abs(backward_buffer[fringe_pos]);
    FLOAT integr_phase = std::arg(backward_buffer[fringe_pos]);
    int max_pos = 0;
    for (size_t i = 1; i < fft_size() + 1; i++) {
      if (std::abs(backward_buffer[i]) > std::abs(backward_buffer[max_pos]))
        max_pos = i;
    }
      
    stats_out << fft_abs << " \t" << fft_phase << " \t"
              << integr_abs << " \t" << integr_phase << " \t"
              << max_pos << std::endl;
  }
#endif // SFXC_WRITE_STATS
#endif // DUMMY_CORRELATION
}

void Correlation_core::integration_normalize(std::vector<Complex_buffer> &integration_buffer) {
  std::vector<FLOAT> norms;
  norms.resize(n_stations());
  memset(&norms[0], 0, norms.size()*sizeof(FLOAT));
  // Average the auto correlations
  for (size_t station=0; station < n_stations(); station++) {
    for (size_t i = 0; i < fft_size() + 1; i++) {
      norms[station] += integration_buffer[station][i].real();
    }
    norms[station] /= (fft_size() / oversamp);
    if(norms[station] < 1)
      norms[station] = 1;

    for (size_t i = 0; i < fft_size() + 1; i++) {
      // imaginary part should be zero!
      integration_buffer[station][i] =
        integration_buffer[station][i].real() / norms[station];
    }
  }

  // Average the cross correlations
  const int64_t total_samples = number_ffts_in_integration * fft_size();
  for (size_t b = n_stations(); b < baselines.size(); b++) {
    SFXC_ASSERT(b < baselines.size());
    std::pair<size_t,size_t> &baseline = baselines[b];
    int32_t *levels1 = statistics[baseline.first]->get_statistics(); 
    int32_t *levels2 = statistics[baseline.second]->get_statistics();
    int64_t n_valid1 =  total_samples - levels1[4]; // levels[4] contain the number of invalid samples
    int64_t n_valid2 =  total_samples - levels2[4];
    FLOAT N1 = n_valid1 > 0? 1 - n_flagged[b].first  * 1. / n_valid1 : 1;
    FLOAT N2 = n_valid2 > 0? 1 - n_flagged[b].second * 1. / n_valid2 : 1;
    FLOAT N = N1 * N2;
    if(N < 0.01) N = 1;
    FLOAT norm = sqrt(N * norms[baseline.first]*norms[baseline.second]);
    for (size_t i = 0 ; i < fft_size() + 1; i++) {
      integration_buffer[b][i] /= norm;
    }
  }
}

void Correlation_core::integration_write(std::vector<Complex_buffer> &integration_buffer, int phase_center, int sourcenr) {

  // Make sure that the input buffers are released
  // This is done by reference counting

  SFXC_ASSERT(writer != boost::shared_ptr<Data_writer>());
  SFXC_ASSERT(integration_buffer.size() == baselines.size());

  // Write the output file index
  {
    size_t nWrite = sizeof(sourcenr);
    writer->put_bytes(nWrite, (char *)&sourcenr);
  }

  int polarisation = 1;
  if (correlation_parameters.polarisation == 'R') {
    polarisation =0;
  } else {
    SFXC_ASSERT(correlation_parameters.polarisation == 'L');
  }

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

  { // Writing the timeslice header
    Output_header_timeslice htimeslice;

    htimeslice.number_baselines = baselines.size();
    htimeslice.integration_slice =
      correlation_parameters.integration_nr + current_integration;
    htimeslice.number_uvw_coordinates = uvw_tables.size();
    htimeslice.number_statistics = input_buffers.size();

   // write the uvw coordinates
    Output_uvw_coordinates uvw[htimeslice.number_uvw_coordinates];
    // We evaluate in the middle of time slice
    Time time = correlation_parameters.start_time + correlation_parameters.integration_time / 2;
    for (size_t station=0; station < uvw_tables.size(); station++){
      double u,v,w;
      uvw_tables[station].get_uvw(phase_center, time, &u, &v, &w);
      uvw[station].station_nr=stream2station[station];
      uvw[station].reserved=0;
      uvw[station].u=u;
      uvw[station].v=v;
      uvw[station].w=w;
    }

    // Write the bit statistics
    Output_header_bitstatistics stats[input_buffers.size()];
    for (size_t stream=0; stream < input_buffers.size(); stream++){
      int station = stream2station[stream]-1;
      int32_t *levels=statistics[stream]->get_statistics();
      if(correlation_parameters.cross_polarize){
        int nstreams=correlation_parameters.station_streams.size();
        stats[stream].polarisation=(stream>=nstreams/2)?1-polarisation:polarisation;
      }else{
        stats[stream].polarisation=polarisation;
      }
      stats[stream].station_nr=station+1;
      stats[stream].sideband = (correlation_parameters.sideband=='L') ? 0 : 1;

      stats[stream].frequency_nr=(unsigned char)correlation_parameters.channel_nr;
      if(statistics[stream]->bits_per_sample==2){
        stats[stream].levels[0]=levels[0];
        stats[stream].levels[1]=levels[1];
        stats[stream].levels[2]=levels[2];
        stats[stream].levels[3]=levels[3];
        stats[stream].n_invalid=levels[4];
      }else{
        stats[stream].levels[0]=0;
        stats[stream].levels[1]=levels[0];
        stats[stream].levels[2]=levels[1];
        stats[stream].levels[3]=0;
        stats[stream].n_invalid=levels[2];
      }
    }

    size_t nWrite = sizeof(htimeslice);
    writer->put_bytes(nWrite, (char *)&htimeslice);
    nWrite=sizeof(uvw);
    writer->put_bytes(nWrite, (char *)&uvw[0]);
    nWrite=sizeof(stats);
    writer->put_bytes(nWrite, (char *)&stats[0]);
  }

  integration_buffer_float.resize(number_channels() + 1);
  size_t n = fft_size() / number_channels();

  Output_header_baseline hbaseline;
  for (size_t i = 0; i < baselines.size(); i++) {
    std::pair<size_t,size_t> &stations = baselines[i];

    for (size_t j = 0; j < number_channels() + 1; j++) {
      integration_buffer_float[j] = integration_buffer[i][j * n];
      for (size_t k = 1; k < n && j < number_channels(); k++)
        integration_buffer_float[j] += integration_buffer[i][j * n + k];
      integration_buffer_float[j] /= n;
    }
    integration_buffer_float[number_channels()] *= n; // Only one point contributes to the last point
    const int64_t total_samples = number_ffts_in_integration * fft_size();
    int32_t *levels = statistics[stations.first]->get_statistics(); // We get the number of invalid samples from the bitstatistics
    if (stations.first == stations.second){
      hbaseline.weight = std::max(total_samples - levels[4], (int64_t) 0);       // The number of good samples
    }else{
      hbaseline.weight = std::max(total_samples - levels[4] - n_flagged[i].first, (int64_t)0);       // The number of good samples
    }
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
    writer->put_bytes((number_channels() + 1) * sizeof(std::complex<float>),
                      ((char*)&integration_buffer_float[0]));
  }
}

void 
Correlation_core::sub_integration(){
  const int current_sub_int = (int) round((double)current_fft / number_ffts_in_sub_integration);
//  const Time tmid = correlation_parameters.start_time +
//                    correlation_parameters.sub_integration_time * current_sub_int -
//                    correlation_parameters.sub_integration_time / 2 ;
  Time tfft(0., correlation_parameters.sample_rate); 
  tfft.inc_samples(fft_size());
  const Time tmid = correlation_parameters.start_time + tfft*(previous_fft+(current_fft-previous_fft)/2.); 

  // Start with the auto correlations
  const int n_fft = fft_size() + 1;
  const int n_phase_centers = phase_centers.size();
  const int n_station = number_input_streams_in_use();
  for (int i = 0; i < n_station; i++) {
    for(int j = 0; j < n_phase_centers; j++){
      for(int k = 0; k < n_fft; k++){
        phase_centers[j][i][k] += accumulation_buffers[i][k];
      }
    }
  }

  const int n_baseline = accumulation_buffers.size();
  for(int i = n_station ; i < n_baseline ; i++){
    std::pair<size_t,size_t> &stations = baselines[i];

    // The pointing center
    for(int j = 0; j < n_fft; j++)
      phase_centers[0][i][j] += accumulation_buffers[i][j];
    // UV shift the additional phase centers
    for(int j = 1; j < n_phase_centers; j++){
      double delay1 = delay_tables[stations.first].delay(tmid);
      double delay2 = delay_tables[stations.second].delay(tmid);
      double ddelay1 = delay_tables[stations.first].delay(j, tmid)-delay1;
      double ddelay2 = delay_tables[stations.second].delay(j, tmid)-delay2;
      double rate1 = delay_tables[stations.first].rate(tmid);
      double rate2 = delay_tables[stations.second].rate(tmid);
      uvshift(accumulation_buffers[i], phase_centers[j][i], ddelay1, ddelay2, rate1, rate2);
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

void
Correlation_core::uvshift(const Complex_buffer &input_buffer, Complex_buffer &output_buffer, double ddelay1, double ddelay2, double rate1, double rate2){
  const int sb = correlation_parameters.sideband == 'L' ? -1 : 1;
  const double base_freq = correlation_parameters.channel_freq;
  const double dfreq = sb * correlation_parameters.sample_rate/ ( 2. * fft_size()); 

  double phi = base_freq * (ddelay1 * (1 - rate1) - ddelay2 * (1 - rate2));
  phi = 2 * M_PI * (phi - floor(phi));
  double delta = 2 * M_PI * dfreq * (ddelay1 * (1 - rate1) - ddelay2 * (1 - rate2));
  double temp=sin(delta/2);
  const double a=2*temp*temp,b=sin(delta);
  double cos_phi, sin_phi;
#ifdef HAVE_SINCOS
  sincos(phi, &sin_phi, &cos_phi);
#else
  sin_phi = sin(phi);
  cos_phi = cos(phi);
#endif 
  const int size = input_buffer.size();
  for (int i = 0; i < size; i++) {
    output_buffer[i] += input_buffer[i] * std::complex<FLOAT>(cos_phi, sin_phi);
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void Correlation_core::add_uvw_table(int sn, Uvw_model &table) {
  if (sn>=uvw_tables.size())
    uvw_tables.resize(sn+1);

  uvw_tables[sn]=table;
}

void Correlation_core::add_delay_table(int sn, Delay_table_akima &table) {
  if (sn>=delay_tables.size())
    delay_tables.resize(sn+1);

  delay_tables[sn]=table;
}

void Correlation_core::add_source_list(const std::map<std::string, int> &sources_){
  sources = sources_;
  split_output = true;
}

void Correlation_core::find_invalid() {
  // First get the number of invalid samples per station
  invalid_elements.resize(0);
  for(int i = 0; i < n_stations(); i++){
    invalid_elements.push_back(&input_buffers[i]->front()->invalid);
  }

  // Now find all the flagged blocks
  for (int b = n_stations(); b < baselines.size(); b++) {
    int s1 = baselines[b].first, s2 = baselines[b].second;
    std::vector<Invalid> *invalid[2] = {invalid_elements[s1], invalid_elements[s2]};
    int index[2] = {0, 0};
    int nflagged[2] = {0, 0};
    int invalid_start[2] = {INT_MAX, INT_MAX};
    int invalid_end[2] = {INT_MAX, INT_MAX};
    const int invalid_size[2] = {invalid[0]->size(), invalid[1]->size()};

    for (int i = 0; i < 2; i++){
      if (invalid_size[i] > 0){
        invalid_start[i] = (*invalid[i])[0].start;
        invalid_end[i] = invalid_start[i] +  (*invalid[i])[index[i]].n_invalid;
        index[i]++;
      }
    }

    int i = invalid_start[0] < invalid_start[1] ? 0 : 1;
    while(invalid_start[i] < INT_MAX){
      if(invalid_start[0] == invalid_start[1]){
        if(invalid_end[0] == invalid_end[1]){
          invalid_start[0] = invalid_end[0] + 1; // invalidate both
          invalid_start[1] = invalid_end[1] + 1;
        }else{
          int j = invalid_end[0] < invalid_end[1] ? 0 : 1;
          invalid_start[1-j] = invalid_end[j];
          invalid_start[j] = invalid_end[j] + 1; // invalidate
        }
      }else if (invalid_end[i] <  invalid_start[1-i]){
        nflagged[1-i] += invalid_end[i] - invalid_start[i];
        invalid_start[i] = invalid_end[i] + 1;
      }else{
        nflagged[1-i] += invalid_start[1-i] - invalid_start[i];
        invalid_start[i] = invalid_start[1-i];
      }
      // update indexes
      for(int j = 0; j < 2; j++){
        if(invalid_start[j] > invalid_end[j]){
          if(index[j] < invalid_size[j]){
            invalid_start[j] =  (*invalid[j])[index[j]].start;
            invalid_end[j] = invalid_start[j] + (*invalid[j])[index[j]].n_invalid; 
            index[j]++;
         }else {
            invalid_start[j] = INT_MAX;
            invalid_end[j] = INT_MAX;
          }
        } 
     }
      i = invalid_start[0] < invalid_start[1] ? 0 : 1;
    }
    n_flagged[b].first += nflagged[0];
    n_flagged[b].second += nflagged[1];
  }
}
