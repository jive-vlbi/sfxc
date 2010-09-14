#include "delay_correction_swapped.h"
#include "config.h"

Delay_correction_swapped::
Delay_correction_swapped(int stream_nr): Delay_correction_base(stream_nr){
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

    double delay = get_delay(current_time + fft_length/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fringe_stopping(&input->data[buf * fft_size()]);

    // zero padding
    IPPS_ZERO_FC((IPP_CPLX_FLOAT*)&frequency_buffer[fft_size()], fft_size());

    // Input is from frequency_buffer
    fractional_bit_shift(output.buffer(),
                         integer_delay,
                         delay_in_samples - integer_delay);

    current_time.inc_samples(fft_size());
    total_ffts++;

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
    IPPS_FFTFWD_CToC_FC_I((IPP_CPLX_FLOAT*)&frequency_buffer[0], plan_t2f, &buffer_t2f[0]);

    // Element 0 and fft_size() should be real numbers
    frequency_buffer[0] = frequency_buffer[0].real() / 2;
    frequency_buffer[fft_size()] = frequency_buffer[fft_size()].real() / 2;
    total_ffts++;
  }

  // 4c) zero the unused subband 

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr = (double)sample_rate() / (2 * fft_size()); // delta frequency
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate(); 
  const double tmp2 = M_PI*(integer_shift&3)/(2*oversamp);
  const double constant_term = -tmp2 + tmp1*0.5*bandwidth();
  const double linear_term = -tmp1*dfr; 

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
    exp_array[i]=std::complex<FLOAT>(cos_phi,sin_phi);
    // the following should be double
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
  IPPS_MUL_FC((IPP_CPLX_FLOAT*)&exp_array[0],(IPP_CPLX_FLOAT*)&frequency_buffer[0],(IPP_CPLX_FLOAT*)&output[0], size);
}

void Delay_correction_swapped::fringe_stopping(FLOAT input[]) {
  const double mult_factor_phi = sideband()*2.0*M_PI; 
  const double center_freq = channel_freq() + sideband()*bandwidth()*0.5;

  // Only compute the delay at integer microseconds
//  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  phi = center_freq * get_delay(current_time);
  int floor_phi = (int)std::floor(phi); // for argument reduction
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

  for (int i = 0; i < fft_size(); i++) {
    exp_array[i]=std::complex<FLOAT>(cos_phi,sin_phi);
    // the following should be double
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
 IPPS_MUL_FFC((IPP_FLOAT*)&input[0], (IPP_CPLX_FLOAT*)&exp_array[0], (IPP_CPLX_FLOAT*)&frequency_buffer[0], fft_size());
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
  oversamp = (int)round(parameters.sample_rate / (2 * parameters.bandwidth));

  current_time = parameters.start_time;
  current_time.set_sample_rate(sample_rate());
  fft_length = Time((double)fft_size() / (sample_rate() / 1000000));

  SFXC_ASSERT(((int64_t)fft_size() * 1000000000) % sample_rate() == 0);

  if (prev_fft_size != fft_size()) {
    exp_array.resize(fft_size() + 1);
    frequency_buffer.resize(2 * fft_size());

    int order=-1, buffersize;
    for(int n = fft_size(); n > 0; order++)
      n/=2;

    int IppStatus = IPPS_FFTINITALLOC_C_FC(&plan_t2f, order+1, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    SFXC_ASSERT(IppStatus == ippStsNoErr);
    IPPS_FFTGETBUFSIZE_C_FC(plan_t2f, &buffersize);
    buffer_t2f.resize(buffersize);
  }
  SFXC_ASSERT(frequency_buffer.size() == 2 * fft_size());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      (int) parameters.integration_time.get_time_usec(),
      parameters.sample_rate,
      parameters.fft_size);
  current_fft = 0;
}
