#include "delay_correction_base.h"

#include "config.h"

Delay_correction_base::Delay_correction_base(int stream_nr)
    : output_buffer(Output_buffer_ptr(new Output_buffer())),
      output_memory_pool(32),current_time(-1), delay_table_set(false),
      stream_nr(stream_nr)
{
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

bool Delay_correction_base::has_work() {
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  if (n_ffts_per_integration == current_fft)
    return false;
  SFXC_ASSERT((current_fft<=n_ffts_per_integration)&&(current_fft>=0))
  return true;
}

Delay_correction_base::Output_buffer_ptr
Delay_correction_base::get_output_buffer() {
  SFXC_ASSERT(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

int Delay_correction_base::length_of_one_fft() {
  return ((int64_t)fft_size() * 1000000) / sample_rate();
}

int Delay_correction_base::sideband() {
  return (correlation_parameters.sideband == 'L' ? -1 : 1);
}
