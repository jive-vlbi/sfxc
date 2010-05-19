#include "delay_correction_swapped.h"

#include "config.h"

Delay_correction_swapped::Delay_correction_swapped(int stream_nr)
  : Delay_correction_base(stream_nr)
{
}

void Delay_correction_swapped::do_task() {
  SFXC_ASSERT(has_work());
  SFXC_ASSERT(current_time >= 0);

  Input_buffer_element input = input_buffer->front_and_pop();
  int nbuffer=input->nfft;
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
    // A factor of 2 for padding
    if (output.size() != 2 * fft_size())
      output.resize(2 * fft_size());
    if (time_buffer.size() != 2 * fft_size())
      time_buffer.resize(2 * fft_size());

    double delay = get_delay(current_time+length_of_one_fft()/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fringe_stopping(&input->data[buf * fft_size()]);

    // zero padding
    for (int i = fft_size(); i < 2 * fft_size(); i++) 
      frequency_buffer[i] = 0;

    // Input is from frequency_buffer
    fractional_bit_shift(output.buffer(),
                         integer_delay,
                         delay_in_samples - integer_delay);

    current_time += length_of_one_fft();

#endif // DUMMY_CORRELATION
  }
  output_buffer->push(cur_output);
}

void Delay_correction_swapped::fractional_bit_shift(std::complex<FLOAT> output[],
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  {
    FFTW_COMPLEX *frequency_buffer_fftw = (FFTW_COMPLEX *)&frequency_buffer[0];
    FFTW_EXECUTE_DFT(plan_t2f,
                     (FFTW_COMPLEX *) &frequency_buffer[0],
                     (FFTW_COMPLEX *) &frequency_buffer[0]);

    // Element 0 and fft_size() should be real numbers
    frequency_buffer[0] = frequency_buffer[0].real() / 2;
    frequency_buffer[fft_size()] = frequency_buffer[fft_size()].real() / 2;
    total_ffts++;
  }

  // 4c) zero the unused subband 
  for (size_t i = fft_size() + 1; i < 2 * fft_size(); i++)
    frequency_buffer[i] = 0.0;

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr = (double)sample_rate() / (2 * fft_size()); // delta frequency
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate(); 
  const double tmp2 = M_PI*(integer_shift&3)/(2*oversamp);
  const double constant_term = tmp2 - tmp1*0.5*bandwidth();
  const double linear_term = tmp1*dfr; 

  // 5b)apply phase correction in frequency range
  const int size = fft_size() + 1;

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
//  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  int64_t time = current_time;
  phi = integer_mult_factor_phi * get_delay(time);
  int floor_phi = (int)std::floor(phi); // for argument reduction
  phi = mult_factor_phi*(phi-floor_phi); 

  { // compute delta_phi
    SFXC_ASSERT(((int64_t)fft_size() * 1000000) % sample_rate() == 0);
    double phi_end = integer_mult_factor_phi *
      get_delay(time + (fft_size() * 1000000) / sample_rate());
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
    // 7)subtract dopplers for the current segment
    frequency_buffer[i] = input[i]*std::complex<FLOAT>(cos_phi,-sin_phi);
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
}

void
Delay_correction_swapped::set_parameters(const Correlation_parameters &parameters) {
  size_t prev_fft_size = fft_size();
  correlation_parameters = parameters;

  int i = 0;
  while ((i < parameters.station_streams.size()) &&
         (parameters.station_streams[i].station_stream != stream_nr))
    i++;
  SFXC_ASSERT(i < parameters.station_streams.size());
  bits_per_sample = parameters.station_streams[i].bits_per_sample;

  nfft_max = std::max(CORRELATOR_BUFFER_SIZE / parameters.fft_size, 1);
  oversamp = round(parameters.sample_rate / (2 * parameters.bandwidth));

  current_time = parameters.start_time*(int64_t)1000;

  SFXC_ASSERT(((int64_t)fft_size() * 1000000000) % sample_rate() == 0);

  if (prev_fft_size != fft_size()) {
    frequency_buffer.resize(2 * fft_size());

    plan_t2f = FFTW_PLAN_DFT_1D(2 * fft_size(),
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                FFTW_BACKWARD, FFTW_MEASURE); //
  }
  SFXC_ASSERT(frequency_buffer.size() == 2 * fft_size());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.fft_size);
  current_fft = 0;
}
