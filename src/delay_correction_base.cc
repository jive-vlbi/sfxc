#include "delay_correction_base.h"

#include "config.h"

const FLOAT sample_value_ms[] = {
                                  -7, -2, 2, 7
                                };
const FLOAT sample_value_m[]  = {
                                  -5, 5
                                };

Delay_correction_base::Delay_correction_base()
    : output_buffer(Output_buffer_ptr(new Output_buffer())),
      output_memory_pool(10),current_time(-1),
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

Delay_correction_base::~Delay_correction_base() {
#if PRINT_TIMER
  int N = number_channels();
  int numiterations = total_ffts;
  double time = delay_timer.measured_time()*1000000;
  PROGRESS_MSG("MFlops: " << 5.0*N*log2(N) * numiterations / (1.0*time));
#endif
}

void Delay_correction_base::connect_to(Input_buffer_ptr new_input_buffer) {
  SFXC_ASSERT(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction_base::set_delay_table(const Delay_table_akima &delay_table_) {
  delay_table_set = true;
  delay_table = delay_table_;
}

double Delay_correction_base::get_delay(int64_t time) {
  SFXC_ASSERT(delay_table_set);
  return delay_table.delay(time);
}

void Delay_correction_base::bit2float(const Input_buffer_element &input,
                                 FLOAT *output_buffer_) {
  FLOAT *output_buffer = output_buffer_;
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

bool Delay_correction_base::has_work() {
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  if (n_ffts_per_integration == current_fft)
    return false;

  return true;
}

Delay_correction_base::Output_buffer_ptr
Delay_correction_base::get_output_buffer() {
  SFXC_ASSERT(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

size_t Delay_correction_base::number_channels() {
  SFXC_ASSERT(correlation_parameters.number_channels >= 0);
  return correlation_parameters.number_channels;
}

size_t Delay_correction_base::size_of_fft() {
  SFXC_ASSERT(correlation_parameters.number_channels >= 0);
  return 2*correlation_parameters.number_channels;
}

int Delay_correction_base::bandwidth() {
  return correlation_parameters.bandwidth;
}
int Delay_correction_base::sample_rate() {
  return correlation_parameters.sample_rate;
}
int Delay_correction_base::length_of_one_fft() {
  return (((int64_t)number_channels())*1000000)/sample_rate();
}
int Delay_correction_base::sideband() {
  SFXC_ASSERT((correlation_parameters.sideband == 'L') ||
              (correlation_parameters.sideband == 'U'));
  return (correlation_parameters.sideband == 'L' ? -1 : 1);
}
int64_t Delay_correction_base::channel_freq() {
  return correlation_parameters.channel_freq;
}
