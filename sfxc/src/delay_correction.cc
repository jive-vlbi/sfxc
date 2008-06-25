#include "delay_correction.h"

#include "config.h"

const FLOAT sample_value_ms[] = {
                                  -7, -2, 2, 7
                                };
const FLOAT sample_value_m[]  = {
                                  -5, 5
                                };

Delay_correction::Delay_correction()
    : output_buffer(Output_buffer_ptr(new Output_buffer())),
    output_memory_pool(10),
    current_time(-1),
delay_table_set(false) {
  // Bit2Float table initialization
  // For 1 bit samples:
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
  PROGRESS_MSG("MFlops: " << 5.0*N*log2(N) * numiterations / (1.0*time));
#endif
}

void Delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  SFXC_ASSERT(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction::set_delay_table(const Delay_table_akima &delay_table_) {
  delay_table_set = true;
  delay_table = delay_table_;
}

double Delay_correction::get_delay(int64_t time) {
  SFXC_ASSERT(delay_table_set);
  return delay_table.delay(time);
}

void Delay_correction::do_task() {
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
  Output_buffer_element output = output_memory_pool.allocate();
  size_t input_size = (input.data().bytes_count()-1)*8/ correlation_parameters.bits_per_sample;


#ifndef DUMMY_CORRELATION
  // A factor of 2 for padding
  if (output->size() != 2*input_size) {
    output->resize(input_size*2);
  }

  bit2float(input, output->buffer() );

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
#endif // DUMMY_CORRELATION

  input_buffer->pop();
  output_buffer->push(output);
}

void Delay_correction::bit2float(const Input_buffer_element &input,
                                 FLOAT* output_buffer_) {
  FLOAT* output_buffer = output_buffer_;
  SFXC_ASSERT(correlation_parameters.bits_per_sample == 2);
  unsigned char * input_data = input->bytes_buffer();

  if (correlation_parameters.bits_per_sample == 2) {
    // First byte:
    memcpy(output_buffer,
           &lookup_table[(int)input_data[0]][(int)input->offset()],
           (4-input->offset())*sizeof(FLOAT));
    output_buffer += 4-input->offset();

    size_t size = input->bytes_count()-1;
    for (size_t byte = 1; byte < size; byte++) {
      memcpy(output_buffer, // byte * 4
             &lookup_table[(int)input_data[byte]][0],
             4*sizeof(FLOAT));
      output_buffer += 4;
    }

    // Last byte:
    memcpy(output_buffer,
           &lookup_table[(int)(unsigned char)input_data[size]][0],
           input->offset()*sizeof(FLOAT));

#ifdef SFXC_INVALIDATE_SAMPLES

    { // zero out the invalid samples
      SFXC_ASSERT(input->invalid_samples_begin >= 0);
      const size_t invalid_samples_begin = input->invalid_samples_begin;
      const size_t invalid_samples_end = invalid_samples_begin + input->nr_invalid_samples;
      SFXC_ASSERT(invalid_samples_begin >= 0);
      SFXC_ASSERT(invalid_samples_begin <= invalid_samples_end);
      SFXC_ASSERT(invalid_samples_end <= number_channels());
      for (size_t i=invalid_samples_begin; i<invalid_samples_end; i++) {
#ifdef SFXC_CHECK_INVALID_SAMPLES
        SFXC_ASSERT(output_buffer_[i] == sample_value_ms[INVALID_PATTERN&3]);
#endif

        output_buffer_[i] = 0;
      }
    }
#endif

  } else {
    std::cout << "Not yet implemented" << std::endl;
    SFXC_ASSERT_MSG(false,
                    "Only 2 bits decoding implemented");
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
    for (size_t i=1; i<number_channels()/2; i++) {
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
  for (int i = 0; i < size; i++) {
    // the following should be double
    double cos_phi, sin_phi;

    // Compute sin_phi=sin(phi); cos_phi = cos(phi);
#ifdef HAVE_SINCOS

    sincos(phi, &sin_phi, &cos_phi);
#else

    sin_phi = sin(phi);
    cos_phi = cos(phi);
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
  const double mult_factor_phi = -sideband()*2.0*M_PI;
  const double integer_mult_factor_phi =
    channel_freq() + sideband()*bandwidth()*0.5;

  // Only compute the delay at integer microseconds
  int n_recompute_delay = sample_rate()/1000000;

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

    // 7)subtract dopplers and put real part in Bufs for the current segment
    output[i] =
      frequency_buffer[i].real()*cos_phi - frequency_buffer[i].imag()*sin_phi;
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
  SFXC_ASSERT(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void
Delay_correction::set_parameters(const Correlation_parameters &parameters) {
  size_t prev_number_channels = number_channels();
  correlation_parameters = parameters;

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
                                FFTW_FORWARD,  FFTW_MEASURE);
  }
  SFXC_ASSERT(frequency_buffer.size() == number_channels());

  n_ffts_per_integration =
    Control_parameters::nr_ffts_per_integration_slice(
      parameters.integration_time,
      parameters.sample_rate,
      parameters.number_channels);
  current_fft = 0;
}

size_t Delay_correction::number_channels() {
  SFXC_ASSERT(correlation_parameters.number_channels >= 0);
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
  SFXC_ASSERT((correlation_parameters.sideband == 'L') ||
              (correlation_parameters.sideband == 'U'));
  return (correlation_parameters.sideband == 'L' ? -1 : 1);
}
int64_t Delay_correction::channel_freq() {
  return correlation_parameters.channel_freq;
}
