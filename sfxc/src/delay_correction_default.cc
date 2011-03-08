#include "delay_correction_default.h"
#include "config.h"

Delay_correction_default::Delay_correction_default(int stream_nr)
  : Delay_correction_base(stream_nr) 
{
}

void Delay_correction_default::do_task() {
  SFXC_ASSERT(has_work());
  SFXC_ASSERT(current_time >= 0);

  Input_buffer_element input = input_buffer->front_and_pop();
  int nbuffer=input->nfft;
  current_fft+=nbuffer;

  // Allocate output buffer
  cur_output=output_memory_pool.allocate();
  int output_stride = fft_size() + 4; // there are fft_size+1 points and each fft should be 16 bytes alligned
  cur_output->stride = output_stride;

  //int output_fft_size = fft_size() + 1;
  if(cur_output->data.size() != nbuffer * output_stride)
    cur_output->data.resize(nbuffer * output_stride);
#ifndef DUMMY_CORRELATION
  // A factor of 2 for padding
  if (time_buffer.size() != 2 * fft_size())
    time_buffer.resize(2 * fft_size());

  for(int buf=0;buf<nbuffer;buf++){
    double delay = get_delay(current_time + fft_length/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fractional_bit_shift(&input->data[buf * fft_size()],
                         integer_delay,
                         delay_in_samples - integer_delay);

    // Input is from frequency_buffer
    fringe_stopping(&time_buffer[0]);

    // Do the final fft from time to frequency:
    // zero out the data for padding
    for (size_t j = fft_size(); j < 2 * fft_size(); j++)
      time_buffer[j] = 0;

    FFTW_EXECUTE_DFT_R2C(plan_t2f_cor,
                         (FLOAT *)&time_buffer[0],
                         (FFTW_COMPLEX *)&cur_output->data[buf * output_stride]);

   current_time.inc_samples(fft_size());
   total_ffts++;
  }
#endif // DUMMY_CORRELATION
  cur_output->invalid = input->invalid;
  output_buffer->push(cur_output);
}

void Delay_correction_default::fractional_bit_shift(FLOAT *input,
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  {
    FFTW_COMPLEX *frequency_buffer_fftw = (FFTW_COMPLEX *)&frequency_buffer[0];
    FFTW_EXECUTE_DFT_R2C(plan_t2f,
                         &input[0],
                         frequency_buffer_fftw);
    total_ffts++;
  }

  // Element 0 and (fft_size() / 2) are real numbers
  frequency_buffer[0] *= 0.5;
  frequency_buffer[fft_size() / 2] *= 0.5; // Nyquist frequency

  // 4c) zero the unused subband (?)
  for (size_t i = (fft_size() / 2) + 1; i < fft_size(); i++)
    frequency_buffer[i] = 0.0;

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
    frequency_buffer[i] *= std::complex<FLOAT>(cos_phi,-sin_phi);
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  FFTW_EXECUTE(plan_f2t);

  total_ffts++;
}

void Delay_correction_default::fringe_stopping(FLOAT output[]) {
  const double mult_factor_phi = -sideband()*2.0*M_PI;
  const double center_freq = channel_freq() + sideband()*bandwidth()*0.5;
  // Only compute the delay at integer microseconds
  //  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  phi = center_freq * get_delay(current_time);
  int floor_phi = (int)std::floor(phi);
  phi = mult_factor_phi*(phi-floor_phi);

  { // compute delta_phi
    SFXC_ASSERT(((int64_t)fft_size() * 1000000) % sample_rate() == 0);
    double phi_end = center_freq *
      get_delay(current_time + fft_length);
    phi_end = mult_factor_phi*(phi_end-floor_phi);

    delta_phi = (phi_end - phi) / fft_size();
  }

  // We perform a recursion for the (co)sines similar to what is done in the fractional bitshift
  double temp=sin(delta_phi/2);
  double a=2*temp*temp,b=sin(delta_phi);
#ifdef HAVE_SINCOS
  sincos(phi, &sin_phi, &cos_phi);
#else
  sin_phi = sin(phi);
  cos_phi = cos(phi);
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
Delay_correction_default::set_parameters(const Correlation_parameters &parameters) {
  size_t prev_fft_size = fft_size();
  correlation_parameters = parameters;

  int i = 0;
  while ((i < parameters.station_streams.size()) &&
         (parameters.station_streams[i].station_stream != stream_nr))
    i++;
  SFXC_ASSERT(i < parameters.station_streams.size());
  bits_per_sample = parameters.station_streams[i].bits_per_sample;

  nfft_max = std::max(CORRELATOR_BUFFER_SIZE / parameters.fft_size, 1);
  oversamp = (int)round(parameters.sample_rate / (2 * parameters.bandwidth));

  current_time = parameters.start_time;
  current_time.set_sample_rate(sample_rate());
  fft_length = Time((double)fft_size() / (sample_rate() / 1000000));

  SFXC_ASSERT(((int64_t)fft_size() * 1000000000) % sample_rate() == 0);

  if (prev_fft_size != fft_size()) {
    frequency_buffer.resize(fft_size());

    Memory_pool_vector_element<FLOAT> input_buffer;
    input_buffer.resize(fft_size());

    plan_t2f = FFTW_PLAN_DFT_R2C_1D(fft_size(),
                                    &input_buffer[0],
                                    (FFTW_COMPLEX *)&frequency_buffer[0],
                                    FFTW_MEASURE);
    plan_f2t = FFTW_PLAN_DFT_1D(fft_size(),
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                FFTW_BACKWARD,  FFTW_MEASURE);

    plan_input_buffer.resize(2 * fft_size());
    plan_output_buffer.resize(fft_size() + 1);
    plan_t2f_cor = FFTW_PLAN_DFT_R2C_1D(2 * fft_size(),
                                  (FLOAT *)plan_input_buffer.buffer(),
                                  (FFTW_COMPLEX *)plan_output_buffer.buffer(),
                                  FFTW_MEASURE);
  }
  SFXC_ASSERT(frequency_buffer.size() == fft_size());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size);

  current_fft = 0;
}
