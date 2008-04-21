#include "delay_correction.h"

#include "config.h"

const FLOAT Delay_correction::maximal_phase_change = 0.2; // 5.7 degrees

Delay_correction::Delay_correction()
    : output_buffer(Output_buffer_ptr(new Output_buffer())),
    output_memory_pool(10),
    current_time(-1),
delay_table_set(false) {
  // Bit2Float table initialization
  const FLOAT sample_value_ms[] = {
                                    -7, -2, 2, 7
                                  };
  // For 1 bit samples:
  //  const FLOAT sample_value_m[]  = {
  //                                    -5, 5
  //                                  };
  for (int i=0; i<256; i++) {
    lookup_table[i][0] = sample_value_ms[(i>>6) & 3];
    lookup_table[i][1] = sample_value_ms[(i>>4) & 3];
    lookup_table[i][2] = sample_value_ms[(i>>2) & 3];
    lookup_table[i][3] = sample_value_ms[i & 3];
  }


}

Delay_correction::~Delay_correction() {
#if PRINT_TIMER
  int N = number_channels();
  int numiterations = total_ffts;
  double time = delay_timer.measured_time()*1000000;
  DEBUG_MSG("MFlops: " << 5.0*N*log2(N) * numiterations / (1.0*time));
#endif
}

void Delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  assert(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction::set_delay_table(const Delay_table_akima &delay_table_) {
  delay_table_set = true;
  delay_table = delay_table_;
}

double Delay_correction::get_delay(int64_t time) {
  assert(delay_table_set);
  return delay_table.delay(time);
}

void Delay_correction::do_task() {
  assert(has_work());
  assert(current_time >= 0);

  if (n_ffts_per_integration == current_fft) {
    assert(current_time/correlation_parameters.integration_time !=
           (current_time+length_of_one_fft())/correlation_parameters.integration_time);

    current_time =
      ((current_time+length_of_one_fft()) /
       correlation_parameters.integration_time)*
      correlation_parameters.integration_time;
    current_fft = 0;
  }
  current_fft++;

  Input_buffer_element input = input_buffer->front_and_pop(); //input_buffer->consume(input_size);
  Output_buffer_element output = output_memory_pool.allocate();
  size_t input_size = (input.data().bytes_count()-1)*8/ correlation_parameters.bits_per_sample;

  // A factor of 2 for padding
  if (output->size() != 2*input_size) {
    output->resize(input_size*2);
  }

  bit2float(input.data().offset(), 
            input.data().bytes_count(), 
            input.data().bytes_buffer(), 
            output->buffer() );


  double delay = get_delay(current_time+length_of_one_fft()/2);
  double delay_in_samples = delay*sample_rate();
  int integer_delay = (int)std::floor(delay_in_samples+.5);


  // Output is in frequency_buffer
  fractional_bit_shift(output->buffer(),
                       integer_delay,
                       delay_in_samples - integer_delay);

  // Input is from frequency_buffer
  fringe_stopping(output->buffer());

  current_time += length_of_one_fft();

  const int n_channels = number_channels();
  for (int i = n_channels; i < 2*n_channels; i++) {
    (*output)[i] = 0;
  }

  input.release();
  output_buffer->push(output);
}

void Delay_correction::bit2float(const unsigned int offset, const unsigned int input_size, const unsigned char* input, FLOAT* output_buffer) {
  assert(correlation_parameters.bits_per_sample == 2);

  if (correlation_parameters.bits_per_sample == 2) {
    // First byte:
    memcpy(output_buffer,
           &lookup_table[(int)input[0]][(int)offset],
           (4-offset)*sizeof(FLOAT));
    output_buffer += 4-offset;

    for (size_t byte = 1; byte < input_size-1; byte++) {
      memcpy(output_buffer, // byte * 4
             &lookup_table[(int)input[byte]][0],
             4*sizeof(FLOAT));
      output_buffer += 4;
    }

    // Last byte:
    memcpy(output_buffer,
           &lookup_table[(int)(unsigned char)input[input_size-1]][0],
           offset*sizeof(FLOAT));
  } else {
    std::cout << "Not yet implemented" << std::endl;
    assert(false);
  }
}


void Delay_correction::fractional_bit_shift(FLOAT input[],
    int integer_shift,
    FLOAT fractional_delay) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  {
    delay_timer.resume();
    //DM replaced: FFTW_EXECUTE_DFT(plan_t2f, (FFTW_COMPLEX *)output, (FFTW_COMPLEX *)output);
    FFTW_COMPLEX *frequency_buffer_fftw = (FFTW_COMPLEX *)&frequency_buffer[0];
    FFTW_EXECUTE_DFT_R2C(plan_t2f,
                         &input[0],
                         frequency_buffer_fftw);
    // Element 0 and number_channels()/2 are real numbers
    for (int i=1; i<number_channels()/2; i++) {
      // frequency_buffer[i] = std::conj(frequency_buffer[i]);
      // This avoids the assignment of the real part
      frequency_buffer_fftw[i][1] = -frequency_buffer_fftw[i][1];
    }
    delay_timer.stop();
    total_ffts++;
  }

  frequency_buffer[0] *= 0.5;
  frequency_buffer[number_channels()/2] *= 0.5;//Nyquist

  // 4c) zero the unused subband (?)
  for (int i=number_channels()/2+1; i<number_channels(); i++) {
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
  for (int i = 0; i < size; i++) {
    // the following should be double
    double cos_phi, sin_phi;

#ifdef HAVE_SINCOS

    sincos(phi, &sin_phi, &cos_phi);
#else

    cos_phi = cos(phi);
    // sin^2(phi) + cos^2(phi) == 1
    int    sign = ( (((int)floor(phi/M_PI))&1 == 1) ? -1 : 1 );
    sin_phi = sign*sqrt(1-cos_phi*cos_phi);
#endif

    frequency_buffer[i] *= std::complex<FLOAT>(cos_phi,sin_phi);

    phi += linear_term;
  }

  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  delay_timer.resume();
  //DM replaced: FFTW_EXECUTE_DFT(plan_f2t, (FFTW_COMPLEX *)output, (FFTW_COMPLEX *)output);
  FFTW_EXECUTE(plan_f2t);
  delay_timer.stop();
  total_ffts++;
}

void Delay_correction::fringe_stopping(FLOAT output[]) {
  double mult_factor_phi =
    -sideband()*2.0*M_PI*(channel_freq() + sideband()*bandwidth()*0.5);

  int64_t time = current_time;

  int64_t delta_time = ((((int64_t)n_recompute_delay)*1000000)/sample_rate());
  if (delta_time < 1)
    delta_time = 1;
  double phi, cosPhi=0, sinPhi=0, deltaCosPhi=0, deltaSinPhi=0;
  // Initialise the end values
  double phi_end = mult_factor_phi * get_delay(time);
  double cosPhi_end = cos(phi_end);
  double sinPhi_end = sin(phi_end);

  for (int i=0; i<number_channels(); i++) {
    if ((i % n_recompute_delay) == 0) {
      phi = phi_end;
      cosPhi = cosPhi_end;
      sinPhi = sinPhi_end;

      phi_end =
        mult_factor_phi * get_delay(time+delta_time);

      if (std::abs(phi_end-phi) < 0.4*maximal_phase_change) {
        // Sampling is too dense
        n_recompute_delay *= 2;
        delta_time = (((int64_t)n_recompute_delay)*1000000)/sample_rate();

        phi_end =
          mult_factor_phi * get_delay(time+delta_time);
      }

      while ((std::abs(phi_end-phi) > maximal_phase_change) &&
             (n_recompute_delay >= 2*sample_rate()/1000000)) {
        // Sampling is not dense enough
        n_recompute_delay /= 2;
        delta_time = (((int64_t)n_recompute_delay)*1000000)/sample_rate();

        phi_end =  mult_factor_phi * get_delay(time+delta_time);
      }

      time += delta_time;

      cosPhi_end = cos(phi_end);
      sinPhi_end = sin(phi_end);

      deltaCosPhi = (cosPhi_end-cosPhi)/n_recompute_delay;
      deltaSinPhi = (sinPhi_end-sinPhi)/n_recompute_delay;


    }

    // 6b)apply normalization and multiply by 2.0
    // NHGK: Why only the real part
    frequency_buffer[i] = std::complex<FLOAT>(2*frequency_buffer[i].real(), 
                                              frequency_buffer[i].imag());

    // 7)subtract dopplers and put real part in Bufs for the current segment
    output[i] =
      frequency_buffer[i].real()*cosPhi - frequency_buffer[i].imag()*sinPhi;
    cosPhi += deltaCosPhi;
    sinPhi += deltaSinPhi;

  }
}

bool Delay_correction::has_work() {
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  if (n_ffts_per_integration == current_fft)
    return false;

  return true;
}

Delay_correction::Output_buffer_ptr
Delay_correction::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void
Delay_correction::set_parameters(const Correlation_parameters &parameters) {
  int prev_number_channels = number_channels();
  correlation_parameters = parameters;

  current_time = parameters.start_time*(int64_t)1000;

  assert((((int64_t)number_channels())*1000000000)%sample_rate() == 0);

  //FLOAT dfr  = 1.0/(n2fftDC*tbs); // delta frequency
  FLOAT dfr  = sample_rate()*1.0/number_channels(); // delta frequency
  freq_scale.resize(number_channels()/2+1);

  for (size_t i=0; i<freq_scale.size(); i++) {
    //frequency scale in the segment
    //fs[jf]=sideband*(jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset());
    freq_scale[i] = sideband()*(i*dfr-0.5*bandwidth());
  }

  n_recompute_delay = sample_rate()/1000000;

  frequency_buffer.resize(number_channels(), 0);

  if (prev_number_channels != number_channels()) {
    //buffer.resize(number_channels());
    Aligned_vector<FLOAT> input_buffer;
    input_buffer.resize(number_channels());

    plan_t2f = FFTW_PLAN_DFT_R2C_1D(number_channels(),
                                    &input_buffer[0],
                                    (FFTW_COMPLEX *)&frequency_buffer[0],
                                    FFTW_MEASURE);
    plan_f2t = FFTW_PLAN_DFT_1D(number_channels(),
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                (FFTW_COMPLEX *)&frequency_buffer[0],
                                FFTW_FORWARD,  FFTW_MEASURE);
  }

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
  current_fft = 0;
}

int Delay_correction::number_channels() {
  return correlation_parameters.number_channels;
}
int Delay_correction::bandwidth() {
  return correlation_parameters.bandwidth;
}
int Delay_correction::sample_rate() {
  return correlation_parameters.sample_rate;
}
int Delay_correction::length_of_one_fft() {
  return (((int64_t)number_channels())*1000000)/sample_rate();
}
int Delay_correction::sideband() {
  assert((correlation_parameters.sideband == 'L') ||
         (correlation_parameters.sideband == 'U'));
  return (correlation_parameters.sideband == 'L' ? -1 : 1);
}
int64_t Delay_correction::channel_freq() {
  return correlation_parameters.channel_freq;
}
