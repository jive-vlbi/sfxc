#include "correlation_core.h"
#include "output_header.h"
#include <utils.h>
#include <complex>
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
  // Process the data of the current fft buffer
  integration_step(accumulation_buffers, nbuffer, stride);
  current_fft += nbuffer;
  for (size_t i = 0; i < number_input_streams(); i++) {
    int stream = station_stream(i);
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
        source_nr = sources[delay_tables[first_stream].get_source(i)]; // FIXME restore
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
    invalid_elements.resize(stream + 1);
  }
  invalid_elements[stream] = invalid_;
}

void Correlation_core::connect_to(size_t stream, bit_statistics_ptr statistics_, Input_buffer_ptr buffer) {
  if (stream >= input_buffers.size()) {
    input_buffers.resize(stream + 1);
    statistics.resize(stream + 1);
  }
  input_buffers[stream] = buffer;
  statistics[stream] = statistics_;
}

void
Correlation_core::set_parameters(const Correlation_parameters &parameters,
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
  for (size_t i = 0; i < number_input_streams(); i++) {
    baselines.push_back(std::pair<size_t, size_t>(i, i));
  }
  // Crosses
  int ref_station = parameters.reference_station;
  for (size_t i = 0; i < number_input_streams(); i++) {
    for (size_t j = i + 1; j < number_input_streams(); j++) {
      int station1 = correlation_parameters.station_streams[i].station_number;
      int station2 = correlation_parameters.station_streams[j].station_number;
      if (station1 == station2)
	continue;
      if (ref_station >= 0 &&
	  ref_station != station1 && ref_station != station2)
	continue;
      if (station1 < station2)
	baselines.push_back(std::make_pair<size_t, size_t>(i, j));
      else
	baselines.push_back(std::make_pair<size_t, size_t>(j, i));
    }
  }
}

void
Correlation_core::
set_data_writer(boost::shared_ptr<Data_writer> writer_) {
  writer = writer_;
}

bool Correlation_core::has_work() {
  for (size_t i = 0; i < number_input_streams(); i++) {
    int stream = station_stream(i);
    if (input_buffers[stream]->empty())
      return false;
  }
  return true;
}

void Correlation_core::integration_initialise() {
  previous_fft = 0;

  if(phase_centers.size() != correlation_parameters.n_phase_centers)
    phase_centers.resize(correlation_parameters.n_phase_centers);

  for (int i = 0; i < phase_centers.size(); i++) {
    if (phase_centers[i].size() != baselines.size()) {
      phase_centers[i].resize(baselines.size());
      for(int j = 0; j < phase_centers[i].size(); j++) {
        phase_centers[i][j].resize(fft_size() + 1);
      }
    }
  }

  for (int i = 0; i < phase_centers.size(); i++) {
    for (int j = 0; j < phase_centers[i].size(); j++) {
      SFXC_ASSERT(phase_centers[i][j].size() == fft_size() + 1);
      size_t size = phase_centers[i][j].size() * sizeof(std::complex<FLOAT>);
      memset(&phase_centers[i][j][0], 0, size);
    }
  }

  if (accumulation_buffers.size() != baselines.size()) {
    accumulation_buffers.resize(baselines.size());
    for (size_t i = 0; i < accumulation_buffers.size(); i++) {
      accumulation_buffers[i].resize(fft_size() + 1);
    }
  }

  SFXC_ASSERT(accumulation_buffers.size() == baselines.size());
  for (size_t i = 0; i < accumulation_buffers.size(); i++) {
    SFXC_ASSERT(accumulation_buffers[i].size() == fft_size() + 1);
    size_t size = accumulation_buffers[i].size() * sizeof(std::complex<FLOAT>);
    memset(&accumulation_buffers[i][0], 0, size);
  }
  memset(&n_flagged[0], 0, sizeof(std::pair<int64_t, int64_t>) * n_flagged.size());
  next_sub_integration = 1;

  fft_f2t.resize(2 * fft_size());
  fft_t2f.resize(2 * number_channels());
  temp_buffer.resize(fft_size() + 1);
  real_buffer.resize(2 * fft_size());

  if (phase_centers.size() > 1)
    create_weights();

  if (fft_size() != number_channels()) {
    create_window();
    create_mask();
  }
}

void Correlation_core::integration_step(std::vector<Complex_buffer> &integration_buffer, int nbuffer, int stride) {
#ifndef DUMMY_CORRELATION
  SFXC_ASSERT(nbuffer * stride <= input_conj_buffers[0].size());

  // Auto correlations
  for (size_t i = 0; i < number_input_streams(); i++) {
    for (size_t buf_idx = 0; buf_idx < nbuffer * stride; buf_idx += stride) {
      // get the complex conjugates of the input
      SFXC_CONJ_FC(&input_elements[i][buf_idx], &(input_conj_buffers[i])[buf_idx], fft_size() + 1);
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[i][buf_idx], 
			  /* in2 */ &input_conj_buffers[i][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
    }
  }

  // Cross correlations
  for (size_t i = number_input_streams(); i < baselines.size(); i++) {
    for (size_t buf_idx = 0; buf_idx < nbuffer * stride; buf_idx += stride) {
      std::pair<size_t, size_t> &baseline = baselines[i];
      SFXC_ASSERT(baseline.first != baseline.second);
      if ((i % 2) == 0 || 1) {
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_elements[baseline.first][buf_idx], 
			  /* in2 */ &input_conj_buffers[baseline.second][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
      } else {
      SFXC_ADD_PRODUCT_FC(/* in1 */ &input_conj_buffers[baseline.first][buf_idx], 
			  /* in2 */ &input_elements[baseline.second][buf_idx],
			  /* out */ &integration_buffer[i][0], fft_size() + 1);
      }
    }
  }
#endif // DUMMY_CORRELATION
}

void Correlation_core::integration_normalize(std::vector<Complex_buffer> &integration_buffer) {
  std::vector<double> norms(number_input_streams());
  memset(&norms[0], 0, norms.size() * sizeof(double));
  // Normalize the auto correlations
  for (size_t i = 0; i < number_input_streams(); i++) {
    for (size_t j = 0; j < fft_size() + 1; j++) {
      norms[i] += integration_buffer[i][j].real();
    }
    norms[i] /= fft_size();
    if (norms[i] < 1)
      norms[i] = 1;

    for (size_t j = 0; j < fft_size() + 1; j++) {
      // imaginary part should be zero!
      integration_buffer[i][j] =
        integration_buffer[i][j].real() / norms[i];
    }
  }

  // Normalize the cross correlations
  const int64_t total_samples = number_ffts_in_integration * fft_size();
  for (size_t i = number_input_streams(); i < baselines.size(); i++) {
    std::pair<size_t, size_t> &baseline = baselines[i];
    int stream1 = station_stream(baseline.first);
    int stream2 = station_stream(baseline.second);
    int32_t *levels1 = statistics[stream1]->get_statistics(); 
    int32_t *levels2 = statistics[stream2]->get_statistics();
    // levels[4] contains the number of invalid samples
    int64_t n_valid1 =  total_samples - levels1[4]; 
    int64_t n_valid2 =  total_samples - levels2[4];
    double N1 = n_valid1 > 0? 1 - n_flagged[i].first  * 1. / n_valid1 : 1;
    double N2 = n_valid2 > 0? 1 - n_flagged[i].second * 1. / n_valid2 : 1;
    double N = N1 * N2;
    if (N < 0.01) N = 1;
    FLOAT norm = sqrt(N * norms[baseline.first] * norms[baseline.second]);
    for (size_t j = 0 ; j < fft_size() + 1; j++) {
      integration_buffer[i][j] /= norm;
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

  int nstreams = number_input_streams();
  std::set<int> stations_set;

  // Initialise with -1
  stations_set.clear();
  for (size_t i = 0; i < nstreams; i++) {
    int station =
      correlation_parameters.station_streams[i].station_number;
    stations_set.insert(station);
  }

  {
    // Timeslice header
    Output_header_timeslice htimeslice;
    htimeslice.number_baselines = baselines.size();
    htimeslice.integration_slice =
      correlation_parameters.integration_nr + current_integration;
    htimeslice.number_uvw_coordinates = stations_set.size();
    htimeslice.number_statistics = nstreams;

    // UVW coordinates
    Output_uvw_coordinates uvw[htimeslice.number_uvw_coordinates];
    stations_set.clear();
    for (size_t i = 0, j = 0; i < nstreams; i++) {
      int stream = station_stream(i);
      int station = station_number(i);
      if (stations_set.count(station) == 0) {
	stations_set.insert(station);
	uvw[j].station_nr = station;
	uvw[j].u = uvw_table[stream][phase_center * 3];
	uvw[j].v = uvw_table[stream][phase_center * 3 + 1];
	uvw[j].w = uvw_table[stream][phase_center * 3 + 2];
	uvw[j].reserved = 0;
	j++;
      }
    }

    // Bit statistics
    Output_header_bitstatistics stats[nstreams];
    for (size_t i = 0; i < nstreams; i++) {
      int stream = station_stream(i);
      int station = station_number(i);
      int32_t *levels = statistics[stream]->get_statistics();
      stats[i].station_nr = station;
      stats[i].sideband = (correlation_parameters.sideband == 'L') ? 0 : 1;
      stats[i].polarisation = (correlation_parameters.station_streams[i].polarisation == 'R') ? 0 : 1;
      stats[i].frequency_nr = (unsigned char)correlation_parameters.frequency_nr;
#ifndef SFXC_ZERO_STATS
      if (statistics[stream]->bits_per_sample == 2) {
	stats[i].levels[0] = levels[0];
	stats[i].levels[1] = levels[1];
	stats[i].levels[2] = levels[2];
	stats[i].levels[3] = levels[3];
	stats[i].n_invalid = levels[4];
      } else {
	stats[i].levels[0] = 0;
	stats[i].levels[1] = levels[0];
	stats[i].levels[2] = levels[1];
	stats[i].levels[3] = 0;
	stats[i].n_invalid = levels[4];
      }
#else
      stats[i].levels[0] = 0;
      stats[i].levels[1] = 0;
      stats[i].levels[2] = 0;
      stats[i].levels[3] = 0;
      stats[i].n_invalid = 0;
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
    std::pair<size_t, size_t> &baseline = baselines[i];
    int stream1 = station_stream(baseline.first);
    int stream2 = station_stream(baseline.second);

    if (fft_size() != number_channels()) {
      if (mask_parameters.normalize) {
	for (size_t j = 0; j < fft_size() + 1; j++) {
	  if (abs(integration_buffer[i][j]) != 0.0)
	    integration_buffer[i][j] /= abs(integration_buffer[i][j]);
	}
      }
      SFXC_MUL_F_FC_I(&mask[0], &integration_buffer[i][0], fft_size() + 1);
      fft_f2t.irfft(&integration_buffer[i][0], &real_buffer[0]);
      real_buffer[number_channels()] =
	(real_buffer[number_channels()] +
	 real_buffer[2 * fft_size() - number_channels()]) / 2;
      for (size_t j = 1; j < number_channels(); j++)
	real_buffer[number_channels() + j] =
	  real_buffer[2 * fft_size() - number_channels() + j];
      SFXC_MUL_F(&real_buffer[0], &window[0], &real_buffer[0],
		 2 * number_channels());
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
    int32_t *levels = statistics[stream1]->get_statistics(); // We get the number of invalid samples from the bitstatistics
    if (stream1 == stream2) {
      hbaseline.weight = std::max(total_samples - levels[4], (int64_t) 0);       // The number of good samples
    } else {
      SFXC_ASSERT(levels[4] >= 0);
      SFXC_ASSERT(n_flagged[i].first >= 0);
      hbaseline.weight = std::max(total_samples - levels[4] - n_flagged[i].first, (int64_t)0);       // The number of good samples
    }
    hbaseline.station_nr1 = station_number(baseline.first);
    hbaseline.station_nr2 = station_number(baseline.second);

    // Polarisation (RCP: 0, LCP: 1)
    hbaseline.polarisation1 = (correlation_parameters.station_streams[baseline.first].polarisation == 'R') ? 0 : 1;
    hbaseline.polarisation2 = (correlation_parameters.station_streams[baseline.second].polarisation == 'R') ? 0 : 1;
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
  for (size_t i = 0; i < number_input_streams(); i++) {
    size_t len = 4 * sizeof(uint8_t) + sizeof(uint64_t) + 4 * sizeof(uint64_t);
    int64_t tsys_on_hi, tsys_on_lo, tsys_off_hi, tsys_off_lo;
    int *tsys;
    char msg[len];
    int pos = 0;

    int stream = station_stream(i);
    uint8_t station = station_number(i);
    uint8_t frequency_number = correlation_parameters.frequency_nr;
    uint8_t sideband = (correlation_parameters.sideband == 'L' ? 0 : 1);
    uint8_t polarisation = (correlation_parameters.station_streams[i].polarisation == 'R' ? 0 : 1);

    tsys = statistics[stream]->get_tsys();
    tsys_on_lo = tsys[0];
    tsys_on_hi = tsys[1];
    tsys_off_lo = tsys[2];
    tsys_off_hi = tsys[3];

    MPI_Pack(&station, 1, MPI_UINT8, msg, len, &pos, MPI_COMM_WORLD);
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
  Time tfft(0., correlation_parameters.sample_rate); 
  tfft.inc_samples(fft_size());
  const Time tmid = correlation_parameters.start_time + tfft*(previous_fft+(current_fft-previous_fft)/2.); 

  // Start with the auto correlations
  const int n_fft = fft_size() + 1;
  const int n_phase_centers = phase_centers.size();
  for (int i = 0; i < number_input_streams(); i++) {
    for (int j = 0; j < n_phase_centers; j++) {
      for (int k = 0; k < n_fft; k++) {
        phase_centers[j][i][k] += accumulation_buffers[i][k];
      }
    }
  }

  for (int i = number_input_streams(); i < baselines.size(); i++) {
    std::pair<size_t, size_t> &baseline = baselines[i];
    int stream1 = station_stream(baseline.first);
    int stream2 = station_stream(baseline.second);

    // The pointing center
    for(int j = 0; j < n_fft; j++)
      phase_centers[0][i][j] += accumulation_buffers[i][j];
    // UV shift the additional phase centers
    for(int j = 1; j < n_phase_centers; j++) {
      double delay1 = delay_tables[stream1].delay(tmid);
      double delay2 = delay_tables[stream2].delay(tmid);
      double ddelay1 = delay_tables[stream1].delay(tmid, j) - delay1;
      double ddelay2 = delay_tables[stream2].delay(tmid, j) - delay2;
      double rate1 = delay_tables[stream1].rate(tmid);
      double rate2 = delay_tables[stream2].rate(tmid);
      uvshift(accumulation_buffers[i], phase_centers[j][i], ddelay1, ddelay2, rate1, rate2);
    }
  }
  // Clear the accumulation buffers
  for (size_t i = 0; i < accumulation_buffers.size(); i++) {
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

  // Compute amplitude scaling
  FLOAT amplitude = 1;
  int lag = abs((int)round((ddelay1 - ddelay2) * correlation_parameters.sample_rate));
  if((lag < fft_size()) && (weights[lag] > 1e-4))
    amplitude = 1. / weights[lag];
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
    output_buffer[i] += amplitude * input_buffer[i] * std::complex<FLOAT>(cos_phi, sin_phi);
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void Correlation_core::add_source_list(const std::map<std::string, int> &sources_){
  sources = sources_;
  split_output = true;
}

void Correlation_core::find_invalid() {
  for (int b = number_input_streams(); b < baselines.size(); b++) {
    int stream1 = station_stream(baselines[b].first);
    int stream2 = station_stream(baselines[b].second);
    // The sample rates of the two stations can potentially be different
    const double scale[2] = {
      correlation_parameters.station_streams[baselines[b].first].sample_rate * 1. /
         correlation_parameters.sample_rate,
      correlation_parameters.station_streams[baselines[b].second].sample_rate * 1. /
         correlation_parameters.sample_rate
    };
    std::vector<Invalid> *invalid[2] = {invalid_elements[stream1], invalid_elements[stream2]};
    int index[2] = {0, 0};
    int nflagged[2] = {0, 0};
    int invalid_start[2] = {INT_MAX, INT_MAX};
    int invalid_end[2] = {INT_MAX, INT_MAX};
    const int invalid_size[2] = {invalid[0]->size(), invalid[1]->size()};

    for (int i = 0; i < 2; i++) {
      if (invalid_size[i] > 0) {
        invalid_start[i] = (int) floor((*invalid[i])[0].start / scale[i]);
        invalid_end[i] = invalid_start[i] + 
                         (int) floor((*invalid[i])[index[i]].n_invalid / scale[i]);
        index[i]++;
      }
    }

    int i = invalid_start[0] < invalid_start[1] ? 0 : 1;
    while (invalid_start[i] < INT_MAX) {
      if (invalid_start[0] == invalid_start[1]) {
        if (invalid_end[0] == invalid_end[1]) {
          invalid_start[0] = invalid_end[0] + 1; // invalidate both
          invalid_start[1] = invalid_end[1] + 1;
        } else {
          int j = invalid_end[0] < invalid_end[1] ? 0 : 1;
          invalid_start[1-j] = invalid_end[j];
          invalid_start[j] = invalid_end[j] + 1; // invalidate
        }
      } else if (invalid_end[i] <  invalid_start[1-i]) {
        nflagged[1-i] += invalid_end[i] - invalid_start[i];
        invalid_start[i] = invalid_end[i] + 1;
      } else {
        nflagged[1-i] += invalid_start[1-i] - invalid_start[i];
        invalid_start[i] = invalid_start[1-i];
      }
      // update indexes
      for (int j = 0; j < 2; j++) {
        if (invalid_start[j] > invalid_end[j]) {
          if (index[j] < invalid_size[j]) {
            invalid_start[j] = (int) floor((*invalid[j])[index[j]].start / scale[j]);
            invalid_end[j] = invalid_start[j] + 
                             (int) floor((*invalid[j])[index[j]].n_invalid / scale[j]);
            index[j]++;
	  } else {
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
  return sin(M_PI * i / n);
}

double
hamming(int n, int i)
{
  return 0.54 - 0.46 * cos(2 * M_PI * i / n);
}

double
hann(int n, int i)
{
  return 0.5 * (1 - cos(2 * M_PI * i / n));
}

double
convolve(double (*f)(int, int), int n, int i)
{
  double sum = 0.0;

  i += n / 2;

  for (int j = 0; j <= n; j++) {
    if ((i - j) < 0)
      continue;
    if ((i - j) > n)
      continue;
    sum += f(n, j) * f(n, i - j);
  }

  return sum;
}

void
Correlation_core::create_weights(){
  double (*f)(int,int);
  if (!weights.empty())
    return;

  switch(correlation_parameters.window){
  case SFXC_WINDOW_NONE:
  case SFXC_WINDOW_RECT:
    // rectangular window (including zero padding)
    f = rect; 
    break;
  case SFXC_WINDOW_COS:
    // Cosine window
    f = cos;
    break;
  case SFXC_WINDOW_HAMMING:
    // Hamming window
    f = hamming;
    break;
  case SFXC_WINDOW_HANN:
    f = hann;
    break;
  default:
    sfxc_abort("Invalid windowing function");
  }
  // Create zero padded window
  const int n = fft_size();
  std::vector<FLOAT> tbuf(4*n);
  std::vector<std::complex<FLOAT> > fbuf(2*n+1), conjbuf(2*n+1);
  weights.resize(4*n);
  for (int i = 0; i < 2*n; i++){
    tbuf[i] = f(2*n-1, i);
  }
  memset(&tbuf[2*n], 0, 2*n*sizeof(FLOAT));
  // Compute auto correlation of windowfunction in Fourier space
  SFXC_FFT fft;
  fft.resize(4*n);
  fft.rfft(&tbuf[0], &fbuf[0]);
  SFXC_CONJ_FC(&fbuf[0], &conjbuf[0], 2*n + 1);
  SFXC_MUL_FC_I(&conjbuf[0], &fbuf[0], 2*n + 1);
  fft.irfft(&fbuf[0], &tbuf[0]);
  // We assume that the window function is symmetric
  weights.resize(n);
  for (int i=0; i<n; i++){
    weights[i] = tbuf[i] / (tbuf[0]);
  }
}

void 
Correlation_core::create_window() {
  const int n = 2 * number_channels();
  const int m = 2 * fft_size();
  double (*f)(int, int);

  if (!window.empty())
    return;

  if (mask_parameters.window.size() > 0) {
    for (int i = 0; i < mask_parameters.window.size(); i++)
      window.push_back(mask_parameters.window[i]);
    SFXC_ASSERT(window.size() == 2 * number_channels());
    return;
  }

  window.resize(n);

  switch(correlation_parameters.window){
  case SFXC_WINDOW_NONE:
  case SFXC_WINDOW_RECT:
    // rectangular window (including zero padding)
    f = rect; 
    break;
  case SFXC_WINDOW_COS:
    // Cosine window
    f = cos;
    break;
  case SFXC_WINDOW_HAMMING:
    // Hamming window
    f = hamming;
    break;
  case SFXC_WINDOW_HANN:
    f = hann;
    break;
  default:
    sfxc_abort("Invalid windowing function");
  }

  window[0] = (m / n) * convolve(f, n, n / 2) / convolve(f, m, m / 2);
  for (int i = 1; i < n / 2; i++)
    window[i] = window[n - i] =
      (m / n) * convolve(f, n, n / 2 + i) / convolve(f, m, m / 2 + i);
  window[n / 2] = convolve(f, n, n) / convolve(f, m, m);
}

void
Correlation_core::create_mask() {
  if (!mask.empty())
    return;

  if (mask_parameters.mask.size() > 0) {
    for (int i = 0; i < mask_parameters.mask.size(); i++)
      mask.push_back(mask_parameters.mask[i]);
    SFXC_ASSERT(mask.size() == fft_size() + 1);
    return;
  }

  mask.assign(fft_size() + 1, 1.0);
}
