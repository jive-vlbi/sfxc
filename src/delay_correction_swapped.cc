#include "delay_correction_swapped.h"

#include "config.h"

Delay_correction_swapped::
Delay_correction_swapped(): Delay_correction_base(),
                            output_buffer(Output_buffer_cmplx_ptr(new Output_buffer_cmplx())),
                            output_memory_pool(10){
}

bool Delay_correction_swapped::has_work() {
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  if (n_ffts_per_integration == current_fft)
    return false;

  return true;
}

void Delay_correction_swapped::do_task() {
  SFXC_ASSERT(has_work());
  SFXC_ASSERT(current_time >= 0);

  if (n_ffts_per_integration == current_fft) {
    SFXC_ASSERT(current_time/correlation_parameters.integration_time !=
                (current_time+length_of_one_fft())/correlation_parameters.integration_time);

    current_time =
      ((current_time+length_of_one_fft()) /
       correlation_parameters.integration_time)*
      correlation_parameters.integration_time;
    current_fft = 0;
  }
  current_fft++;

  Input_buffer_element &input = input_buffer->front();
  Output_buffer_cmplx_element output = output_memory_pool.allocate();
  size_t input_size = (input.data().bytes_count()-1)*8/ correlation_parameters.bits_per_sample;


#ifndef DUMMY_CORRELATION
  const int n_channels = number_channels();
  // A factor of 2 for padding
  if (output->size() != 2*input_size)
    output->resize(input_size*2);
  if (time_buffer.size() != 2*input_size)
    time_buffer.resize(input_size*2);

  bit2float(input, &time_buffer[0] );

  double delay = get_delay(current_time+length_of_one_fft()/2);
  double delay_in_samples = delay*sample_rate();
  int integer_delay = (int)std::floor(delay_in_samples+.5);

  // Output is in frequency_buffer
  fringe_stopping(&time_buffer[0]);

  // zero padding
  for (int i = n_channels; i < 2*n_channels; i++) 
    frequency_buffer[i] = 0;

  // Input is from frequency_buffer
  fractional_bit_shift(output->buffer(),
                       integer_delay,
                       delay_in_samples - integer_delay);

  current_time += length_of_one_fft();

#endif // DUMMY_CORRELATION

  input_buffer->pop();
  output_buffer->push(output);
}

void Delay_correction_swapped::fractional_bit_shift(std::complex<FLOAT> output[],
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  const int n_channels = number_channels();
  {
    delay_timer.resume();
    //DM replaced: FFTW_EXECUTE_DFT(plan_t2f, (FFTW_COMPLEX *)output, (FFTW_COMPLEX *)output);
    FFTW_COMPLEX *frequency_buffer_fftw = (FFTW_COMPLEX *)&frequency_buffer[0];
    FFTW_EXECUTE_DFT(plan_t2f,
                     (FFTW_COMPLEX *) &frequency_buffer[0],
                     (FFTW_COMPLEX *) &frequency_buffer[0]);

  // Element 0 and number_channels() should be real numbers
    frequency_buffer[0]=frequency_buffer[0].real()/2;
    frequency_buffer[n_channels]=frequency_buffer[n_channels].real()/2;
    delay_timer.stop();
    total_ffts++;
  }

  // 4c) zero the unused subband 
  for (size_t i=n_channels+1; i<2*n_channels; i++) {
    frequency_buffer[i] = 0.0;
  }

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr  = sample_rate()*1.0/(2*n_channels); // delta frequency 
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate(); 
  const double tmp2 = 0.5*M_PI*(integer_shift&3);/* was: / ovrfl */ 
  const double constant_term = tmp2 - sideband()*tmp1*0.5*bandwidth();
  const double linear_term = tmp1*sideband()*dfr; 

  // 5b)apply phase correction in frequency range
  const int size = n_channels+1;

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
    output[i] = std::conj(frequency_buffer[i]*std::complex<FLOAT>(cos_phi,sin_phi)); 

    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void Delay_correction_swapped::fringe_stopping(FLOAT input[]) {
  const double mult_factor_phi = sideband()*2.0*M_PI; 
  const double integer_mult_factor_phi =
    channel_freq() + sideband()*bandwidth()*0.5;

  // Only compute the delay at integer microseconds
  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  int64_t time = current_time;
  phi = integer_mult_factor_phi * get_delay(time);
  int floor_phi = (int)std::floor(phi); // for argument reduction
  phi = mult_factor_phi*(phi-floor_phi); 

  { // compute delta_phi
    SFXC_ASSERT((number_channels()*1000000)%sample_rate() == 0);
    double phi_end = integer_mult_factor_phi *
                     get_delay(time + (number_channels()*1000000)/sample_rate());
    phi_end = mult_factor_phi*(phi_end-floor_phi);

    delta_phi = (phi_end-phi)*n_recompute_delay/number_channels(); 
  }
  for (size_t i=0; i<number_channels(); i++) {
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    if ((i%n_recompute_delay)==0) {
#ifdef HAVE_SINCOS
      sincos(phi, &sin_phi, &cos_phi);
#else
      sin_phi = sin(phi);
      cos_phi = cos(phi);
#endif
      phi += delta_phi;
    }

    // 7)subtract dopplers for the current segment
    frequency_buffer[i] = input[i]*std::complex<FLOAT>(cos_phi,-sin_phi);
  }
}

Delay_correction_swapped::Output_buffer_cmplx_ptr
Delay_correction_swapped::get_output_buffer() {
  SFXC_ASSERT(output_buffer != Output_buffer_cmplx_ptr());
  return output_buffer;
}

void
Delay_correction_swapped::set_parameters(const Correlation_parameters &parameters) {
  size_t prev_number_channels = number_channels();
  correlation_parameters = parameters;

  current_time = parameters.start_time*(int64_t)1000;

  SFXC_ASSERT((((int64_t)number_channels())*1000000000)%sample_rate() == 0);

  if (prev_number_channels != number_channels()) {
    frequency_buffer.resize(2*number_channels());

    plan_t2f = FFTW_PLAN_DFT_1D(2*number_channels(),
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                FFTW_BACKWARD, FFTW_MEASURE); //
  }
  SFXC_ASSERT(frequency_buffer.size() == 2*number_channels());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
  current_fft = 0;
}
