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
  while (output_buffer->size() > 0)
    output_buffer->pop();
}

void Delay_correction::do_task() {
  SFXC_ASSERT(has_work());
  Input_buffer_element input = input_buffer->front_and_pop();
  int nbuffer=input->nfft;
  current_fft+=nbuffer;
  //Time ttt = Time(55870, 79405.0);
  Time ttt = Time(55870, 0.);
  if((RANK_OF_NODE==-10)&&(current_time >= ttt)) std::cerr << "delay : " << n_ffts_per_integration << " / " << current_fft << "\n";
  // Allocate output buffer
  Output_buffer_element cur_output = output_memory_pool.allocate();
  if (cur_output->data.size() != nbuffer * fft_size())
    cur_output->data.resize(nbuffer * fft_size());
  FLOAT *output_data = &cur_output->data[0];
#ifndef DUMMY_CORRELATION
  for(int buf=0;buf<nbuffer;buf++){
    double delay = get_delay(current_time + fft_length/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fractional_bit_shift(&input->data[buf * fft_size()],
                         integer_delay,
                         delay_in_samples - integer_delay);

    // Input is from frequency_buffer
    fringe_stopping(&output_data[buf * fft_size()]);
    // Flip sidebandedness, if needed
    if (correlation_parameters.sideband != correlation_parameters.station_streams[stream_idx].sideband)
      SFXC_MUL_F(&output_data[buf * fft_size()], &flip[0], &output_data[buf * fft_size()], fft_size());

    current_time.inc_samples(fft_size());
    total_ffts++;
  }
 
#endif // DUMMY_CORRELATION
  output_buffer->push(cur_output);
}

void Delay_correction::empty_output_queue(){
  while (output_buffer->size() > 0)
    output_buffer->pop();
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
  unsigned int state = (unsigned int) floor(current_time.get_time_usec()); // FIXME remove
  for (size_t i = 0; i < fft_size(); i++) {
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    // 7)subtract dopplers and put real part in Bufs for the current segment
    output[i] =
      frequency_buffer[i].real()*cos_phi + frequency_buffer[i].imag()*sin_phi;
    // FIXME remove this debug
    /*int64_t usec = (int64_t) round(current_time.get_time_usec());
    if((usec%1000 >=100) && (usec%1000 <200)){
      double val = rand_r(&state)*1./RAND_MAX;
      if (val > 0.8347256)
        output[i] = 7.;
      else if (val > 0.5)
        output[i] = 2.;
      else if (val > 0.1652743)
        output[i] = -2.;
      else
        output[i] = -7.;
    }
    else
      output[i] = 0;*/
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void
Delay_correction::set_parameters(const Correlation_parameters &parameters) {
  empty_output_queue();
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

  current_time = parameters.stream_start;
  current_time.set_sample_rate(sample_rate());
  fft_length = Time((double)fft_size() / (sample_rate() / 1000000));

  SFXC_ASSERT(((int64_t)fft_size() * 1000000000) % sample_rate() == 0);

  exp_array.resize(fft_size());
  frequency_buffer.resize(fft_size());

  fft_t2f.resize(fft_size());
  fft_f2t.resize(fft_size());
  create_flip();

  SFXC_ASSERT(parameters.fft_size_correlation >= parameters.fft_size_delaycor);
  n_ffts_per_integration =
    (parameters.station_streams[stream_idx].sample_rate / parameters.sample_rate) *
    parameters.slice_size / parameters.fft_size_delaycor;
  current_fft = 0;
  tbuf_start = 0;
  tbuf_end = 0;
/*  std::cerr << RANK_OF_NODE << " : fft_size_dedispersion = " 
            << correlation_parameters.fft_size_dedispersion 
            << ", nfft_max " << nfft_max<< "\n";*/
}

void Delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  SFXC_ASSERT(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction::set_delay_table(const Delay_table_akima &table) {
  delay_table_set = true;
  delay_table = table;
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

// It is possible to flip the sidebandedness of a subband by flipping
// the sign of every other sample in the time domain.  This function
// constructs a vector to do this.

void
Delay_correction::create_flip() {
  const int n = fft_size();
  flip.resize(n);
  for (int i = 0; i < n; i++)
    flip[i] = ((i % 2) == 0) ? 1 : -1;
}
