#include "delay_correction_default.h"
#include "config.h"

Delay_correction_default::Delay_correction_default(int stream_nr)
  : Delay_correction_base(stream_nr) 
{
}

void Delay_correction_default::do_task() {
  SFXC_ASSERT(has_work());
  SFXC_ASSERT(current_time >= 0);

  Input_buffer_element &input = input_buffer->front();
  int input_size = (input->data.size() * 8) / bits_per_sample;
  int nbuffer=input_size/number_channels();
  current_fft+=nbuffer;

  // Allocate output buffer
  cur_output=output_memory_pool.allocate();
  if(cur_output.data().size() != nbuffer){
    // Avoid resizes later on
    if(cur_output.data().capacity() < nfft_max)
      cur_output.data().reserve(nfft_max);

    cur_output.data().resize(nbuffer);
  }

  for(int buf=0;buf<nbuffer;buf++)
  {
    Output_data &output = cur_output.data()[buf];
#ifndef DUMMY_CORRELATION
    const int n_channels = number_channels();
    // A factor of 2 for padding
    if (output.size() != 2*n_channels)
      output.resize(n_channels*2);
    if (time_buffer.size() != 2*n_channels)
      time_buffer.resize(n_channels*2);

    //convert the input samples to floating point
    bit2float(input, buf, &time_buffer[0]);
    double delay = get_delay(current_time+length_of_one_fft()/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fractional_bit_shift(&time_buffer[0],
                         integer_delay,
                         delay_in_samples - integer_delay);

    // Input is from frequency_buffer
    fringe_stopping(&time_buffer[0]);

    current_time += length_of_one_fft();

    // Do the final fft from time to frequency:
    // zero out the data for padding
    for (size_t j=size_of_fft()/2; j<size_of_fft(); j++) {
         time_buffer[j] = 0;
    }

    FFTW_EXECUTE_DFT_R2C(plan_t2f_cor,
                         (FLOAT *)&time_buffer[0],
                         (FFTW_COMPLEX *)output.buffer());

    total_ffts++;
#endif // DUMMY_CORRELATION
  }
  input_buffer->pop();
  output_buffer->push(cur_output);
}

void Delay_correction_default::fractional_bit_shift(FLOAT input[],
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  {
    //DM replaced: FFTW_EXECUTE_DFT(plan_t2f, (FFTW_COMPLEX *)output, (FFTW_COMPLEX *)output);
    FFTW_COMPLEX *frequency_buffer_fftw = (FFTW_COMPLEX *)&frequency_buffer[0];
    FFTW_EXECUTE_DFT_R2C(plan_t2f,
                         &input[0],
                         frequency_buffer_fftw);
    total_ffts++;
  }
  // Element 0 and number_channels()/2 are real numbers
  frequency_buffer[0] *= 0.5;
  frequency_buffer[number_channels()/2] *= 0.5;//Nyquist frequency

  // 4c) zero the unused subband (?)
  for (size_t i=number_channels()/2+1; i<number_channels(); i++) {
    frequency_buffer[i] = 0.0;
  }

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr  = sample_rate()*1.0/number_channels(); // delta frequency
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate();
  const double tmp2 = 0.5*M_PI*(integer_shift&3);/* was: / ovrfl */
  const double constant_term = tmp2 - sideband()*tmp1*0.5*bandwidth();
  const double linear_term = tmp1*sideband()*dfr;

  // 5b)apply phase correction in frequency range
  const int size = number_channels()/2+1;

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
  //DM replaced: FFTW_EXECUTE_DFT(plan_f2t, (FFTW_COMPLEX *)output, (FFTW_COMPLEX *)output);
  FFTW_EXECUTE(plan_f2t);
  total_ffts++;
}

void Delay_correction_default::fringe_stopping(FLOAT output[]) {
  const double mult_factor_phi = -sideband()*2.0*M_PI;
  const double integer_mult_factor_phi =
    channel_freq() + sideband()*bandwidth()*0.5;

  // Only compute the delay at integer microseconds
 //  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  int64_t time = current_time;
  phi = integer_mult_factor_phi * get_delay(time);
  int floor_phi = (int)std::floor(phi);
  phi = mult_factor_phi*(phi-floor_phi);

  { // compute delta_phi
    SFXC_ASSERT((number_channels()*1000000)%sample_rate() == 0);
    double phi_end = integer_mult_factor_phi *
                     get_delay(time + (number_channels()*1000000)/sample_rate());
    phi_end = mult_factor_phi*(phi_end-floor_phi);

    delta_phi = (phi_end-phi)/number_channels();
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

  for (size_t i=0; i<number_channels(); i++) {
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
  size_t prev_number_channels = number_channels();
  correlation_parameters = parameters;
  bits_per_sample = correlation_parameters.station_streams[stream_nr].bits_per_sample;
  int fft_size = (parameters.number_channels * bits_per_sample) / 8;
  nfft_max = INPUT_NODE_PACKET_SIZE/fft_size;

  current_time = parameters.start_time*(int64_t)1000;

  SFXC_ASSERT((((int64_t)number_channels())*1000000000)%sample_rate() == 0);

  if (prev_number_channels != number_channels()) {
    frequency_buffer.resize(number_channels());

    Memory_pool_vector_element<FLOAT> input_buffer;
    input_buffer.resize(number_channels());

    plan_t2f = FFTW_PLAN_DFT_R2C_1D(number_channels(),
                                    &input_buffer[0],
                                    (FFTW_COMPLEX *)&frequency_buffer[0],
                                    FFTW_MEASURE);
    plan_f2t = FFTW_PLAN_DFT_1D(number_channels(),
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                FFTW_BACKWARD,  FFTW_MEASURE);

    plan_input_buffer.resize(size_of_fft());
    plan_output_buffer.resize(size_of_fft()/2+1);
    plan_t2f_cor = FFTW_PLAN_DFT_R2C_1D(size_of_fft(),
                                  (FLOAT *)plan_input_buffer.buffer(),
                                  (FFTW_COMPLEX *)plan_output_buffer.buffer(),
                                  FFTW_MEASURE);
  }
  SFXC_ASSERT(frequency_buffer.size() == number_channels());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
  current_fft = 0;
}
