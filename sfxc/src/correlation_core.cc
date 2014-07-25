#include "correlation_core.h"
#include "output_header.h"
#include <utils.h>
#include <set>

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
  integration_step(accumulation_buffers, nbuffer, stride);
  current_fft += nbuffer;
  for (size_t i=0, nstreams=number_input_streams_in_use(); i<nstreams; i++){
    int stream = streams_in_scan[i];
    input_buffers[stream]->pop();
  }
 
  if (current_fft == number_ffts_in_integration) {
    PROGRESS_MSG("node " << node_nr_ << ", "
                 << current_fft << " of " << number_ffts_in_integration);

    sub_integration();
    find_invalid();
    for(int i = 0 ; i < phase_centers.size(); i++){
      integration_normalize(phase_centers[i]);
      int source_nr;
      if(split_output){
        delay_tables[0].goto_scan(correlation_parameters.start_time);
        source_nr = sources[delay_tables[0].get_source(i)];
      }
      else if(correlation_parameters.pulsar_binning){
        source_nr = 1; // Source 0 is reserved for of-pulse data
      }else{
        source_nr = 0;
      }
      integration_write(phase_centers[i], i, source_nr);
    }
    tsys_write();
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

void Correlation_core::connect_to(size_t stream, std::vector<Invalid> *invalid_) {
  if (stream >= invalid_elements.size()) {
    invalid_elements.resize(stream+1);
  }
  invalid_elements[stream] = invalid_;
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

  // If the relevant correlation parameters change, clear the window
  // vector.  It will be recreated when the next integration starts.
  if (parameters.number_channels != correlation_parameters.number_channels ||
      parameters.fft_size_correlation != correlation_parameters.fft_size_correlation ||
      parameters.window != correlation_parameters.window)
    window.clear();

  correlation_parameters = parameters;
  oversamp = (int) round(parameters.sample_rate / (2 * parameters.bandwidth));

  create_baselines(parameters);
  if (input_elements.size() != number_input_streams_in_use()) {
    input_elements.resize(number_input_streams_in_use());
  }
  if (input_conj_buffers.size() != number_input_streams_in_use()) {
    input_conj_buffers.resize(number_input_streams_in_use());
  }
  n_flagged.resize(baselines.size());
  get_input_streams();
}

void
Correlation_core::get_input_streams(){
  // Get a sorted list of input streams
  std::set<int> streams_set;
  for(int i=0; i < correlation_parameters.station_streams.size(); i++){
    int stream = correlation_parameters.station_streams[i].station_stream;
    streams_set.insert(stream);
  }
  streams_in_scan.resize(0);
  for(std::set<int>::iterator it = streams_set.begin(); it != streams_set.end(); it++){
    streams_in_scan.push_back(*it);
  }
}

void
Correlation_core::create_baselines(const Correlation_parameters &parameters){
  number_ffts_in_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size_correlation); 
  // One less because of the overlapping windows
  if(parameters.window != SFXC_WINDOW_NONE)
    number_ffts_in_integration -= 1;

  number_ffts_in_sub_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.sub_integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size_correlation);
  baselines.clear();
  // Autos
  for (size_t sn = 0 ; sn < number_input_streams_in_use(); sn++) {
    baselines.push_back(std::pair<int,int>(sn,sn));
  }
  // Crosses
  int ref_station = parameters.reference_station;
  if (parameters.cross_polarize) {
    SFXC_ASSERT(number_input_streams_in_use() % 2 == 0);
    size_t n_st_2 = number_input_streams_in_use()/2;
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
      for (int sn = 0 ; sn < (int)number_input_streams_in_use(); sn++) {
        if (sn != ref_station) {
          baselines.push_back(std::pair<int,int>(sn,ref_station));
        }
      }
    } else { // No reference station
      // no cross polarisation without a reference station

      for (size_t sn = 0 ; sn < number_input_streams_in_use() - 1; sn++) {
        for (size_t sno = sn + 1; sno < number_input_streams_in_use() ; sno ++) {
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
    int stream = streams_in_scan[i];
    if (input_buffers[stream]->empty())
      return false;
  }
  return true;
}

void Correlation_core::integration_initialise() {
  previous_fft = 0;

  if(phase_centers.size() != correlation_parameters.n_phase_centers)
    phase_centers.resize(correlation_parameters.n_phase_centers);

  for(int i = 0 ; i < phase_centers.size() ; i++){
    if (phase_centers[i].size() != baselines.size()){
      phase_centers[i].resize(baselines.size());
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

  fft_f2t.resize(2 * fft_size());
  fft_t2f.resize(2 * number_channels());
  temp_buffer.resize(fft_size() + 1);
  real_buffer.resize(2 * fft_size());

  if (fft_size() != number_channels())
    create_window();
}

void Correlation_core::integration_step(std::vector<Complex_buffer> &integration_buffer, int nbuffer, int stride) {
#ifndef DUMMY_CORRELATION
  // do the correlation
  SFXC_ASSERT(nbuffer * stride <= input_conj_buffers[0].size());
  for (size_t i = 0; i < number_input_streams_in_use(); i++) {
    for (size_t buf_idx = 0; buf_idx < nbuffer * stride; buf_idx += stride) {
      // get the complex conjugates of the input
      SFXC_CONJ_FC(&input_elements[i][buf_idx], &(input_conj_buffers[i])[buf_idx], fft_size() + 1);
      // Auto correlation
      std::pair<size_t,size_t> &stations = baselines[i];
      SFXC_ASSERT(stations.first == stations.second);
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[stations.first][buf_idx], 
			  /* in2 */ &input_conj_buffers[stations.first][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
    }
  }

  for (size_t i = number_input_streams_in_use(); i < baselines.size(); i++) {
    for (size_t buf_idx = 0; buf_idx < nbuffer * stride; buf_idx += stride) {
      // Cross correlations
      std::pair<size_t,size_t> &stations = baselines[i];
      SFXC_ASSERT(stations.first != stations.second);
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[stations.first][buf_idx], 
			  /* in2 */ &input_conj_buffers[stations.second][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
    }
  }
#endif // DUMMY_CORRELATION
}

void Correlation_core::integration_normalize(std::vector<Complex_buffer> &integration_buffer) {
  std::vector<double> norms(number_input_streams_in_use());
  memset(&norms[0], 0, norms.size()*sizeof(double));
  // Normalize the auto correlations
  for (size_t station=0; station < number_input_streams_in_use(); station++) {
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

  // Normalize the cross correlations
  const int64_t total_samples = number_ffts_in_integration * fft_size();
  for (size_t b = number_input_streams_in_use(); b < baselines.size(); b++) {
    SFXC_ASSERT(b < baselines.size());
    std::pair<size_t,size_t> &baseline = baselines[b];
    int station1 = streams_in_scan[baseline.first];
    int station2 = streams_in_scan[baseline.second];
    int32_t *levels1 = statistics[station1]->get_statistics(); 
    int32_t *levels2 = statistics[station2]->get_statistics();
    int64_t n_valid1 =  total_samples - levels1[4]; // levels[4] contain the number of invalid samples
    int64_t n_valid2 =  total_samples - levels2[4];
    double N1 = n_valid1 > 0? 1 - n_flagged[b].first  * 1. / n_valid1 : 1;
    double N2 = n_valid2 > 0? 1 - n_flagged[b].second * 1. / n_valid2 : 1;
    double N = N1 * N2;
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

  int nstreams = correlation_parameters.station_streams.size();
  int nstations = nstreams;
  if (correlation_parameters.cross_polarize) 
    nstations /= 2;
  std::vector<int> stream2station;

  {
    // initialise with -1
    stream2station.resize(input_buffers.size(), -1);

    for (size_t i=0; i < nstreams; i++) {
      size_t station_stream =
        correlation_parameters.station_streams[i].station_stream;
      stream2station[station_stream] =
        correlation_parameters.station_streams[i].station_number;
    }
  }

  { // Writing the timeslice header
    Output_header_timeslice htimeslice;

    htimeslice.number_baselines = baselines.size();
    htimeslice.integration_slice =
      correlation_parameters.integration_nr + current_integration;
    htimeslice.number_uvw_coordinates = nstations;
    htimeslice.number_statistics = nstreams;

   // write the uvw coordinates
    Output_uvw_coordinates uvw[htimeslice.number_uvw_coordinates];
    // We evaluate in the middle of time slice
    Time time = correlation_parameters.start_time + correlation_parameters.integration_time / 2;
    for (size_t i=0; i < nstations; i++){
      double u,v,w;
      int stream = streams_in_scan[i];
      uvw_tables[stream].get_uvw(phase_center, time, &u, &v, &w);
      uvw[i].station_nr=stream2station[stream];
      uvw[i].reserved=0;
      uvw[i].u=u;
      uvw[i].v=v;
      uvw[i].w=w;
    }

    // Write the bit statistics
    Output_header_bitstatistics stats[nstreams];
    for (size_t i=0; i < nstreams; i++){
      int stream = streams_in_scan[i];
      int station = stream2station[stream]-1;
      int32_t *levels=statistics[stream]->get_statistics();
      if(correlation_parameters.cross_polarize){
        int nstreams_max = input_buffers.size();
        stats[i].polarisation=(stream>=nstreams_max/2)?1-polarisation:polarisation;
      }else{
        stats[i].polarisation=polarisation;
      }
      stats[i].station_nr=station+1;
      stats[i].sideband = (correlation_parameters.sideband=='L') ? 0 : 1;
      stats[i].frequency_nr = (unsigned char)correlation_parameters.frequency_nr;
#ifndef SFXC_ZERO_STATS
      if(statistics[stream]->bits_per_sample==2){
        stats[i].levels[0]=levels[0];
        stats[i].levels[1]=levels[1];
        stats[i].levels[2]=levels[2];
        stats[i].levels[3]=levels[3];
        stats[i].n_invalid=levels[4];
      }else{
        stats[i].levels[0]=0;
        stats[i].levels[1]=levels[0];
        stats[i].levels[2]=levels[1];
        stats[i].levels[3]=0;
        stats[i].n_invalid=levels[4];
      }
#else
      stats[i].levels[0]=0;
      stats[i].levels[1]=0;
      stats[i].levels[2]=0;
      stats[i].levels[3]=0;
      stats[i].n_invalid=0;
#endif
    }

    size_t nWrite = sizeof(htimeslice);
    writer->put_bytes(nWrite, (char *)&htimeslice);
    nWrite=sizeof(uvw);
    writer->put_bytes(nWrite, (char *)&uvw[0]);
    nWrite=sizeof(stats);
    writer->put_bytes(nWrite, (char *)&stats[0]);
  }

  SFXC_ASSERT(fft_size() >= number_channels());
  integration_buffer_float.resize(number_channels() + 1);

  Output_header_baseline hbaseline;
  for (size_t i = 0; i < baselines.size(); i++) {
    std::pair<size_t,size_t> &inputs = baselines[i];
    int station1 = streams_in_scan[inputs.first];
    int station2 = streams_in_scan[inputs.second];

    if (fft_size() != number_channels()) {
      fft_f2t.irfft(&integration_buffer[i][0], &real_buffer[0]);
      for (size_t j = 0; j < number_channels(); j++)
	real_buffer[number_channels() + j] =
	  real_buffer[2 * fft_size() - number_channels() + j];
      SFXC_MUL_F(&real_buffer[0], &window[0], &real_buffer[0], 2 * number_channels());
      fft_t2f.rfft(&real_buffer[0], &temp_buffer[0]);
      for (size_t j = 0; j < number_channels() + 1; j++) {
	integration_buffer_float[j] = temp_buffer[j];
	integration_buffer_float[j] /= (2 * fft_size());
      }
    } else {
      for (size_t j = 0; j < number_channels() + 1; j++)
	integration_buffer_float[j] = integration_buffer[i][j];
    }

    const int64_t total_samples = number_ffts_in_integration * fft_size();
    int32_t *levels = statistics[station1]->get_statistics(); // We get the number of invalid samples from the bitstatistics
    if (station1 == station2){
      hbaseline.weight = std::max(total_samples - levels[4], (int64_t) 0);       // The number of good samples
    }else{
      SFXC_ASSERT(levels[4] >= 0);
      SFXC_ASSERT(n_flagged[i].first >= 0);
      hbaseline.weight = std::max(total_samples - levels[4] - n_flagged[i].first, (int64_t)0);       // The number of good samples
    }
    // Station number in the vex-file
    hbaseline.station_nr1 = stream2station[station1];
    // Station number in the vex-file
    hbaseline.station_nr2 = stream2station[station2];

    // Polarisation for the first station
    SFXC_ASSERT((polarisation == 0) || (polarisation == 1)); // (RCP: 0, LCP: 1)
    hbaseline.polarisation1 = polarisation;
    hbaseline.polarisation2 = polarisation;
    if (correlation_parameters.cross_polarize) {
      if (inputs.first >= nstations)
        hbaseline.polarisation1 = 1-polarisation;
      if (inputs.second >= nstations)
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
    hbaseline.frequency_nr = (unsigned char)correlation_parameters.frequency_nr;
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
Correlation_core::tsys_write() {
  std::vector<int> stream2station;
  stream2station.resize(input_buffers.size(), -1);
  for (size_t i = 0; i < number_input_streams_in_use(); i++) {
    size_t station_stream =
      correlation_parameters.station_streams[i].station_stream;
    stream2station[station_stream] =
      correlation_parameters.station_streams[i].station_number;
  }

  for (size_t i = 0; i < number_input_streams_in_use(); i++) {
    size_t len = 4 * sizeof(uint8_t) + sizeof(uint64_t) + 4 * sizeof(uint64_t);
    int64_t tsys_on_hi, tsys_on_lo, tsys_off_hi, tsys_off_lo;
    int *tsys;
    char msg[len];
    int pos = 0;

    int stream = streams_in_scan[i];
    uint8_t station_number = stream2station[stream];
    uint8_t frequency_number = correlation_parameters.frequency_nr;
    uint8_t sideband = (correlation_parameters.sideband == 'L' ? 0 : 1);
    uint8_t polarisation = (correlation_parameters.polarisation == 'R' ? 0 : 1);
    if (correlation_parameters.cross_polarize && stream >= input_buffers.size() / 2)
      polarisation = 1 - polarisation;

    tsys = statistics[stream]->get_tsys();
    tsys_on_lo = tsys[0];
    tsys_on_hi = tsys[1];
    tsys_off_lo = tsys[2];
    tsys_off_hi = tsys[3];

    MPI_Pack(&station_number, 1, MPI_UINT8, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&frequency_number, 1, MPI_UINT8, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&sideband, 1, MPI_UINT8, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&polarisation, 1, MPI_UINT8, msg, len, &pos, MPI_COMM_WORLD);
    uint64_t ticks = correlation_parameters.start_time.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&tsys_on_lo, 1, MPI_INT64, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&tsys_on_hi, 1, MPI_INT64, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&tsys_off_lo, 1, MPI_INT64, msg, len, &pos, MPI_COMM_WORLD);
    MPI_Pack(&tsys_off_hi, 1, MPI_INT64, msg, len, &pos, MPI_COMM_WORLD);

    MPI_Send(msg, pos, MPI_PACKED, RANK_OUTPUT_NODE, MPI_TAG_OUTPUT_NODE_WRITE_TSYS, MPI_COMM_WORLD);
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
    std::pair<size_t,size_t> &inputs = baselines[i];
    int station1 = streams_in_scan[inputs.first];
    int station2 = streams_in_scan[inputs.second];

    // The pointing center
    for(int j = 0; j < n_fft; j++)
      phase_centers[0][i][j] += accumulation_buffers[i][j];
    // UV shift the additional phase centers
    for(int j = 1; j < n_phase_centers; j++){
      double delay1 = delay_tables[station1].delay(tmid);
      double delay2 = delay_tables[station2].delay(tmid);
      double ddelay1 = delay_tables[station1].delay(tmid, j)-delay1;
      double ddelay2 = delay_tables[station2].delay(tmid, j)-delay2;
      double rate1 = delay_tables[station1].rate(tmid);
      double rate2 = delay_tables[station2].rate(tmid);
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
  const double dfreq = correlation_parameters.sample_rate/ ( 2. * fft_size()); 

  double phi = base_freq * (ddelay1 * (1 - rate1) - ddelay2 * (1 - rate2));
  phi = 2 * M_PI * sb * (phi - floor(phi));
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

  uvw_tables[sn].add_scans(table);
}

void Correlation_core::add_delay_table(int sn, Delay_table_akima &table) {
  if (sn>=delay_tables.size())
    delay_tables.resize(sn+1);

  delay_tables[sn].add_scans(table);
}

void Correlation_core::add_source_list(const std::map<std::string, int> &sources_){
  sources = sources_;
  split_output = true;
}

void Correlation_core::find_invalid() {
 for (int b = number_input_streams_in_use(); b < baselines.size(); b++) {
    int s1 = streams_in_scan[baselines[b].first];
    int s2 = streams_in_scan[baselines[b].second];
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
    SFXC_ASSERT(nflagged[0] >= 0);
    SFXC_ASSERT(nflagged[1] >= 0);
    n_flagged[b].first += nflagged[0];
    n_flagged[b].second += nflagged[1];
  }
}

double
rect(int n, int i)
{
  if (i >= n / 4 && i < (3 * n) / 4)
    return 1.0;
  else
    return 0.0;
}

double
cos(int n, int i)
{
  return sin(M_PI * i /(n - 1));
}

double
hamming(int n, int i)
{
  return 0.54 - 0.46 * cos(2 * M_PI * i / (n - 1));
}

double
hann(int n, int i)
{
  return 0.5 * (1 - cos(2 * M_PI * i / (n - 1)));
}

double
convolve(double (*f)(int, int), int n, int i)
{
  double sum = 0.0;

  i += n / 2;
  i -= 1;

  for (int j = 0; j <= i; j++) {
    if ((i - j) >= n)
      continue;
    sum += f(n, j) * f(n, i - j);
  }

  return sum;
}

void 
Correlation_core::create_window() {
  const int n = 2 * number_channels();
  const int m = 2 * fft_size();

  if (!window.empty())
    return;

  window.resize(n);

  switch(correlation_parameters.window){
  case SFXC_WINDOW_NONE:
  case SFXC_WINDOW_RECT:
    // rectangular window (including zero padding)
    for (int i = 0; i < n; i++)
      window[(i + n / 2) % n] =
	(m / n) * convolve(rect, n, i) / convolve(rect, m, ((m - n) / 2) + i);
    break;
  case SFXC_WINDOW_COS:
    // Cosine window
    for (int i = 0; i < n; i++)
      window[(i + n / 2) % n] =
	(m / n) * convolve(cos, n, i) / convolve(cos, m, ((m - n) / 2) + i);
    break;
  case SFXC_WINDOW_HAMMING:
    // Hamming window
    for (int i = 0; i < n; i++)
      window[(i + n / 2) % n] =
	(m / n) * convolve(hamming, n, i) / convolve(hamming, m, ((m - n) / 2) + i);
    break;
  case SFXC_WINDOW_HANN:
    for (int i = 0; i < n; i++)
      window[(i + n / 2) % n] =
	(m / n) * convolve(hann, n, i) / convolve(hann, m, ((m - n) / 2) + i);
    break;
  default:
    sfxc_abort("Invalid windowing function");
  }
}
