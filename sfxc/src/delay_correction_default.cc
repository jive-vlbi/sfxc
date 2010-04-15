#include "delay_correction_default.h"
#include "config.h"

Delay_correction_default::
Delay_correction_default(int stream_nr): Delay_correction_base(stream_nr){
}

void Delay_correction_default::do_task() {
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
  for(int buf=0;buf<nbuffer;buf++){
    Output_data &output = cur_output.data()[buf];
#ifndef DUMMY_CORRELATION
    const int n_channels = number_channels();
    // A factor of 2 for padding
    if (output.size() != 2*n_channels)
      output.resize(n_channels*2);
    if (time_buffer.size() != 2*n_channels)
      time_buffer.resize(n_channels*2);

    double delay = get_delay(current_time+length_of_one_fft()/2);
    double delay_in_samples = delay*sample_rate();
    int integer_delay = (int)std::floor(delay_in_samples+.5);

    // Output is in frequency_buffer
    fractional_bit_shift(&input->data[buf*n_channels],
                         integer_delay,
                         delay_in_samples - integer_delay);

    // Input is from frequency_buffer
    fringe_stopping(&time_buffer[0]);

    current_time += length_of_one_fft();

    // Do the final fft from time to frequency:
    // zero out the data for padding
    IPPS_ZERO_F((IPP_FLOAT*)&time_buffer[size_of_fft()/2],size_of_fft()/2);

    IPPS_FFTFWD_RTOCSS_F(&time_buffer[0], (IPP_FLOAT*)output.buffer(), plan_t2f_cor, &buffer_t2f_cor[0]);

    total_ffts++;
#endif // DUMMY_CORRELATION
  }
  output_buffer->push(cur_output);
}

void Delay_correction_default::fractional_bit_shift(FLOAT *input,
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  {
    IPPS_FFTFWD_RTOCSS_F(&input[0], (IPP_FLOAT*)&frequency_buffer[0], plan_t2f, &buffer_t2f[0]);
    total_ffts++;
  }

  // Element 0 and number_channels()/2 are real numbers
  frequency_buffer[0] *= 0.5;
  frequency_buffer[number_channels()/2] *= 0.5;//Nyquist frequency

  // 4c) zero the unused subband (?)
  IPPS_ZERO_FC((IPP_CPLX_FLOAT*)&frequency_buffer[number_channels()/2+1],number_channels()/2-1);

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  // the following should be double
  const double dfr  = sample_rate()*1.0/number_channels(); // delta frequency
  const double tmp1 = -2.0*M_PI*fractional_delay/sample_rate();
  const double tmp2 = M_PI*(integer_shift&3)/(2*oversamp);
  const double constant_term = - tmp2 + sideband()*tmp1*0.5*bandwidth();
  const double linear_term = -tmp1*sideband()*dfr;

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
    exp_array[i]=std::complex<FLOAT>(cos_phi,sin_phi);
    // the following should be double
    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
    temp=sin_phi-(a*sin_phi-b*cos_phi);
    cos_phi=cos_phi-(a*cos_phi+b*sin_phi);
    sin_phi=temp;
  }
  IPPS_MUL_FC_I((IPP_CPLX_FLOAT*)&exp_array[0],(IPP_CPLX_FLOAT*)&frequency_buffer[0],size);

  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  IPPS_FFTINV_CToC_FC_I((IPP_CPLX_FLOAT*)&frequency_buffer[0], plan_f2t, &buffer_f2t[0]);
  total_ffts++;
}

void Delay_correction_default::fringe_stopping(FLOAT output[]) {
  const double mult_factor_phi = -sideband()*2.0*M_PI;
  const double center_freq = channel_freq() + sideband()*bandwidth()*0.5;

  // Only compute the delay at integer microseconds
 //  int n_recompute_delay = sample_rate()/1000000;

  double phi, delta_phi, sin_phi, cos_phi;
  int64_t time = current_time;
  phi = center_freq * get_delay(time);
  int floor_phi = (int)std::floor(phi);
  phi = mult_factor_phi*(phi-floor_phi);

  { // compute delta_phi
    SFXC_ASSERT((number_channels()*1000000LL)%sample_rate() == 0);
    double phi_end = center_freq *
                     get_delay(time + (number_channels()*1000000LL)/sample_rate());
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
  int i=0;
  while ((i<correlation_parameters.station_streams.size())&&
         (correlation_parameters.station_streams[i].station_stream!=stream_nr))
    i++;
  SFXC_ASSERT(i<correlation_parameters.station_streams.size());
  bits_per_sample = correlation_parameters.station_streams[i].bits_per_sample;
  nfft_max = std::max(CORRELATOR_BUFFER_SIZE/parameters.number_channels,1);
  oversamp = round(correlation_parameters.sample_rate/(2*correlation_parameters.bandwidth));

  current_time = parameters.start_time*(int64_t)1000;

  SFXC_ASSERT((((int64_t)number_channels())*1000000000)%sample_rate() == 0);

  if (prev_number_channels != number_channels()) {
    exp_array.resize(number_channels());
    frequency_buffer.resize(number_channels());

    Memory_pool_vector_element<FLOAT> input_buffer;
    input_buffer.resize(number_channels());

    int order=-1;
    for(int n=number_channels();n>0;order++)
      n/=2;

    int IppStatus = IPPS_FFTINITALLOC_R_F(&plan_t2f, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    SFXC_ASSERT(IppStatus == ippStsNoErr);
    int buffersize;
    IPPS_FFTGETBUFSIZE_R_F(plan_t2f, &buffersize);
    buffer_t2f.resize(buffersize);

    IppStatus = IPPS_FFTINITALLOC_C_FC(&plan_f2t, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    SFXC_ASSERT(IppStatus == ippStsNoErr);
    IPPS_FFTGETBUFSIZE_C_FC(plan_f2t, &buffersize);
    buffer_f2t.resize(buffersize);
    
    IppStatus = IPPS_FFTINITALLOC_R_F(&plan_t2f_cor, order+1, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    SFXC_ASSERT(IppStatus == ippStsNoErr);
    IPPS_FFTGETBUFSIZE_R_F(plan_t2f_cor, &buffersize);
    buffer_t2f_cor.resize(buffersize);
  }
  SFXC_ASSERT(frequency_buffer.size() == number_channels());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
  current_fft = 0;
}
