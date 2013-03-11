#include "delay_correction.h"
#include "sfxc_math.h"
#include "config.h"

Delay_correction::Delay_correction(int stream_nr_)
    : output_buffer(Output_buffer_ptr(new Output_buffer())),
      output_memory_pool(32),current_time(-1), delay_table_set(false),
      stream_nr(stream_nr_), stream_idx(-1)
{
}

Delay_correction::~Delay_correction() {
#if PRINT_TIMER
  int N = number_channels();
  int numiterations = total_ffts;
  double time = delay_timer.measured_time()*1000000;
  PROGRESS_MSG("MFlops: " << 5.0*N*log2(N) * numiterations / (1.0*time));
#endif
}

void Delay_correction::do_task() {
  SFXC_ASSERT(has_work());
  Input_buffer_element input = input_buffer->front_and_pop();
  int nbuffer=input->nfft;
  current_fft+=nbuffer;
  // Allocate output buffer
  int output_stride =  fft_cor_size()/2 + 4; // there are fft_size+1 points and each fft should be 16 bytes alligned
  cur_output = output_memory_pool.allocate();
  cur_output->stride = output_stride;
  // The windowing touches each block twice, so the last block needs to be preserved
  int nfft_cor = (nbuffer * fft_size() + tbuf_end - tbuf_start) / (fft_rot_size() / 2) - 1;
  if (cur_output->data.size() != nfft_cor * output_stride)
    cur_output->data.resize(nfft_cor * output_stride);
#ifndef DUMMY_CORRELATION
  size_t tbuf_size = time_buffer.size();
  for(int buf=0;buf<nbuffer;buf++){
    double delay = get_delay(current_time + fft_length/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fractional_bit_shift(&input->data[buf * fft_size()],
                         integer_delay,
                         delay_in_samples - integer_delay);

    // Input is from frequency_buffer
    fringe_stopping(&time_buffer[tbuf_end%tbuf_size]);
    tbuf_end += fft_size();

    current_time.inc_samples(fft_size());
    total_ffts++;
  }
 
  for(int i=0; i<nfft_cor; i++){
    // apply window function
    size_t eob = tbuf_size - tbuf_start%tbuf_size; // how many samples to end of buffer
    size_t nsamp = std::min(eob, fft_rot_size());
    SFXC_MUL_F(&time_buffer[tbuf_start%tbuf_size], &window[0], &temp_buffer[0], nsamp);
    if(nsamp < fft_rot_size())
      SFXC_MUL_F(&time_buffer[0], &window[nsamp], &temp_buffer[nsamp], fft_rot_size() - nsamp);
    tbuf_start += fft_rot_size()/2;
    SFXC_ASSERT(tbuf_start < tbuf_end);
    // Do the final fft from time to frequency
    if (correlation_parameters.sideband != correlation_parameters.station_streams[stream_idx].sideband)
      SFXC_MUL_F(&temp_buffer[0], &flip[0], &temp_buffer[0], fft_rot_size());
    fft_t2f_cor.rfft(&temp_buffer[0], &temp_fft_buffer[0]);
    memcpy(&cur_output->data[i * output_stride], &temp_fft_buffer[temp_fft_offset], output_stride * sizeof(std::complex<FLOAT>));
  }
  if ((current_fft == n_ffts_per_integration) && (correlation_parameters.window == SFXC_WINDOW_NONE)){
    // Also get the last fft
    size_t eob = tbuf_size - tbuf_start%tbuf_size; // how many samples to end of buffer
    size_t nsamp = std::min(eob, fft_rot_size()/2);
    memcpy(&temp_buffer[0], &time_buffer[tbuf_start%tbuf_size], nsamp * sizeof(FLOAT));
    if(nsamp < fft_rot_size())
      memcpy(&temp_buffer[nsamp], &time_buffer[0], (fft_rot_size()/2 - nsamp) * sizeof(FLOAT));
    memset(&temp_buffer[fft_rot_size()/2], 0, sizeof(FLOAT) * fft_rot_size()/2);
    // Do the final fft from time to frequency
    if (correlation_parameters.sideband != correlation_parameters.station_streams[stream_idx].sideband)
      SFXC_MUL_F(&temp_buffer[0], &flip[0], &temp_buffer[0], fft_rot_size());
    cur_output->data.resize((nfft_cor+1) * output_stride);
    fft_t2f_cor.rfft(&temp_buffer[0], &temp_fft_buffer[0]);
    memcpy(&cur_output->data[nfft_cor * output_stride], &temp_fft_buffer[temp_fft_offset], output_stride * sizeof(std::complex<FLOAT>));
  }
#endif // DUMMY_CORRELATION
  if(nfft_cor > 0){
    output_buffer->push(cur_output);
  }
}

void Delay_correction::fractional_bit_shift(FLOAT *input,
    int integer_shift,
    double fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  fft_t2f.rfft(&input[0], &frequency_buffer[0]);
  total_ffts++;

  // Element 0 and (fft_size() / 2) are real numbers
  frequency_buffer[0] *= 0.5;
  frequency_buffer[fft_size() / 2] *= 0.5; // Nyquist frequency

  // 4c) zero the unused subband (?)
  SFXC_ZERO_FC(&frequency_buffer[(fft_size() / 2) + 1], (fft_size() / 2) - 1);

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr  = (double)sample_rate() / fft_size(); // delta frequency
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate();
  const double tmp2 = M_PI*(integer_shift&3)/(2*oversamp);
  const double constant_term = tmp2 -tmp1*0.5*bandwidth();
  const double linear_term = tmp1*dfr;

  // 5b)apply phase correction in frequency range
  const int size = (fft_size() / 2) + 1;

  double phi = constant_term;
  // in the loop we calculate sin(phi) and cos(phi) with phi=contant_term + i*linear_term
  // This we do efficiently using a recurrence relation
  // sin(t+delta)=sin(t)-[a*sin(t)-b*cos(t)] ; cos(t+delta)=cos(t)-[a*cos(t)+b*sin(t)]
  // a=2*sin^2(delta/2) ; b=sin(delta)
  double temp=sin(linear_term/2);
  double a=2*temp*temp,b=sin(linear_term);
  double cos_phi, sin_phi;
#ifdef HAVE_SINCOS
  sincos(phi, &sin_phi, &cos_phi);
#else
  sin_phi = sin(phi);
  cos_phi = cos(phi);
#endif
  for (int i = 0; i < size; i++) {
    // the following should be double
    exp_array[i] = std::complex<FLOAT>(cos_phi,-sin_phi);
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
  SFXC_MUL_FC_I(&exp_array[0], &frequency_buffer[0], size);

  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  fft_f2t.ifft(&frequency_buffer[0], &frequency_buffer[0]);

  total_ffts++;
}

void Delay_correction::fringe_stopping(FLOAT output[]) {
  const double mult_factor_phi = -sideband()*2.0*M_PI;
  const double center_freq = channel_freq() + sideband()*bandwidth()*0.5;
  // Only compute the delay at integer microseconds
  //  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  phi = center_freq * get_delay(current_time) + get_phase(current_time) / mult_factor_phi;
  double floor_phi = std::floor(phi);
  phi = mult_factor_phi*(phi-floor_phi);

  { // compute delta_phi
    SFXC_ASSERT(((int64_t)fft_size() * 1000000) % sample_rate() == 0);
    double phi_end = center_freq * get_delay(current_time + fft_length) + 
                     get_phase(current_time + fft_length) / mult_factor_phi;
    phi_end = mult_factor_phi*(phi_end-floor_phi);

    delta_phi = (phi_end - phi) / fft_size();
  }

  // We use a constant amplitude factor over the fft
  double amplitude = get_amplitude(current_time + fft_length/2);
  // We perform a recursion for the (co)sines similar to what is done in the fractional bitshift
  double temp=sin(delta_phi/2);
  double a=2*temp*temp,b=sin(delta_phi);
#ifdef HAVE_SINCOS
  sincos(phi, &sin_phi, &cos_phi);
  sin_phi *= amplitude;
  cos_phi *= amplitude;
#else
  sin_phi = amplitude * sin(phi);
  cos_phi = amplitude * cos(phi);
#endif

  for (size_t i = 0; i < fft_size(); i++) {
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    // 7)subtract dopplers and put real part in Bufs for the current segment
    output[i] =
      frequency_buffer[i].real()*cos_phi + frequency_buffer[i].imag()*sin_phi;

    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void
Delay_correction::set_parameters(const Correlation_parameters &parameters) {
  stream_idx = 0;
  while ((stream_idx < parameters.station_streams.size()) &&
         (parameters.station_streams[stream_idx].station_stream != stream_nr))
    stream_idx++;
  if (stream_idx == parameters.station_streams.size()) {
    // Data stream is not participating in current time slice
    return;
  }

  bits_per_sample = parameters.station_streams[stream_idx].bits_per_sample;
  correlation_parameters = parameters;
  oversamp = (int)round(sample_rate() / (2 * bandwidth()));

  current_time = parameters.start_time;
  current_time.set_sample_rate(sample_rate());
  fft_length = Time((double)fft_size() / (sample_rate() / 1000000));

  SFXC_ASSERT(((int64_t)fft_size() * 1000000000) % sample_rate() == 0);

  size_t nfft_min = std::max(2*fft_rot_size()/fft_size(), (size_t)1);
  size_t nfft_max = std::max(CORRELATOR_BUFFER_SIZE / fft_size(), nfft_min) + nfft_min;
  time_buffer.resize(nfft_max * fft_size());

  exp_array.resize(fft_size());
  frequency_buffer.resize(fft_size());
  temp_buffer.resize(fft_rot_size());
  temp_fft_buffer.resize(fft_rot_size()/2 + 4);

  fft_t2f.resize(fft_size());
  fft_f2t.resize(fft_size());
  fft_t2f_cor.resize(fft_rot_size());
  create_window();
  create_flip();

  // Calculate the offset into temp_fft_buffer where we can find the
  // spectral points that we want to correlate.
  int64_t delta, freq = channel_freq();
  if (parameters.sideband != parameters.station_streams[stream_idx].sideband)
    freq += sideband() * bandwidth();
  if (parameters.sideband == 'L')
    delta = freq - parameters.channel_freq;
  else
    delta = parameters.channel_freq - freq;
  SFXC_ASSERT(delta >= 0);
  temp_fft_offset = (fft_cor_size() / 2) * delta  / parameters.bandwidth;

  SFXC_ASSERT(parameters.fft_size_correlation >= parameters.fft_size_delaycor);
  n_ffts_per_integration =
    (parameters.station_streams[stream_idx].bandwidth / parameters.bandwidth) *
    (parameters.fft_size_correlation / parameters.fft_size_delaycor) *
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size_correlation);

  current_fft = 0;
  tbuf_start = 0;
  tbuf_end = 0;
}

void Delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  SFXC_ASSERT(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction::set_delay_table(const Delay_table_akima &table) {
  delay_table_set = true;
  delay_table.add_scans(table);
}

double Delay_correction::get_delay(Time time) {
  SFXC_ASSERT(delay_table_set);
  return delay_table.delay(time);
}

double Delay_correction::get_phase(Time time) {
  SFXC_ASSERT(delay_table_set);
  return delay_table.phase(time);
}

double Delay_correction::get_amplitude(Time time) {
  SFXC_ASSERT(delay_table_set);
  return delay_table.amplitude(time);
}

bool Delay_correction::has_work() {
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  if (n_ffts_per_integration == current_fft)
    return false;
  SFXC_ASSERT((current_fft<=n_ffts_per_integration)&&(current_fft>=0))
  return true;
}

Delay_correction::Output_buffer_ptr
Delay_correction::get_output_buffer() {
  SFXC_ASSERT(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

int Delay_correction::sideband() {
  return (correlation_parameters.station_streams[stream_idx].sideband == 'L' ? -1 : 1);
}

void 
Delay_correction::create_window(){
  const int n = fft_rot_size();
  window.resize(n);
  switch(correlation_parameters.window){
  case SFXC_WINDOW_NONE:
    //  Identical to the case without windowing
    for (int i=0; i<n/2; i++)
      window[i] = 1;
    for (int i = n/2; i < n; i++)
      window[i] = 0;
    break;
  case SFXC_WINDOW_RECT:
    // rectangular window (including zero padding)
    for (int i=0; i<n/4; i++)
      window[i] = 0;
    for (int i = n/4; i < 3*n/4; i++)
      window[i] = 1;
    for (int i = 3*n/4 ; i < n ; i++)
      window[i] = 0;
    break;
  case SFXC_WINDOW_COS:
    // Cosine window
    for (int i=0; i<n; i++){
      window[i] = sin(M_PI * i /(n-1));
    }
    break;
  case SFXC_WINDOW_HAMMING:
    // Hamming window
    for (int i=0; i<n; i++){
      window[i] = 0.54 - 0.46 * cos(2*M_PI*i/(n-1));
    }
    break;
  case SFXC_WINDOW_HANN:
    // Hann window
    for (int i=0; i<n; i++){
      window[i] = 0.5 * (1 - cos(2*M_PI*i/(n-1)));
    }
    break;
  default:
    sfxc_abort("Invalid windowing function");
  }
}

// It is possible to flip the sidebandedness of a subband by flipping
// the sign of every other sample in the time domain.  This function
// constructs a vector to do this.

void
Delay_correction::create_flip() {
  const int n = fft_rot_size();
  flip.resize(n);
  for (int i = 0; i < n; i++)
    flip[i] = ((i % 2) == 0) ? 1 : -1;
}
