#include "input_data_format_reader_tasklet.h"

Input_data_format_reader_tasklet::
Input_data_format_reader_tasklet(Data_format_reader_ptr reader,
                     Data_frame &data)
    : memory_pool_(10), stop_time(-1),
    n_bytes_per_input_word(reader->bytes_per_input_word()) {

  SFXC_ASSERT(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  reader_ = reader;

  assert(data.mark5_data.size() > 0);

  *input_element_ = data;

  current_time = reader_->get_current_time();

#ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream chexid;
  std::stringstream monid;

  inputid << "inputnode" << RANK_OF_NODE;
  chexid << inputid.str() << "_mark5a_reader";
  monid << chexid.str() << "_monitor_speed";

  monitor_.init(monid.str(), 1000, "stats/");
  monitor_.add_property(inputid.str(), "is_a", "inputnode");
  monitor_.add_property(inputid.str(), "has", chexid.str() );
  monitor_.add_property(chexid.str(), "is_a", "mark5a_reader");
  monitor_.add_property(chexid.str(), "has", monid.str() );

#endif //RUNTIME_STATISTIC
}
void
Input_data_format_reader_tasklet::
do_task() {
#ifdef RUNTIME_STATISTIC
  monitor_.begin_measure();
#endif // RUNTIME_STATISTIC

  SFXC_ASSERT(has_work());

  push_element();
  allocate_element();

  if (reader_->eof()) {
    randomize_block();
    current_time += reader_->time_between_headers();
  } else if (current_time < reader_->get_current_time()) {
    randomize_block();
    current_time += reader_->time_between_headers();
  } else {

    if (!reader_->read_new_block(*input_element_)) {
      randomize_block();
    }
    current_time = reader_->get_current_time();
  }
  input_element_->start_time = current_time;

#ifdef RUNTIME_STATISTIC
  monitor_.end_measure(reader->size_data_block());
#endif // RUNTIME_STATISTIC
}

bool
Input_data_format_reader_tasklet::
has_work() {
  if (memory_pool_.empty()) {
    return false;
  }
  if (stop_time <= current_time) {
    return false;
  }

  return true;
}
void
Input_data_format_reader_tasklet::
allocate_element() {
  SFXC_ASSERT(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
}
int
Input_data_format_reader_tasklet::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  int64_t new_time =
    reader_->goto_time(*input_element_, us_time);
  SFXC_ASSERT(new_time == reader_->get_current_time());

  // Set the current time to the actual time in the data stream.
  // Might not be the requested time, if no data is available
  current_time = new_time;
  
  input_element_->start_time = current_time;

  if (us_time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << us_time << "us.");
    DEBUG_MSG("Current time is              " << current_time << "us.");
    DEBUG_MSG("Time found is                " << new_time << "us.");
  }

  return current_time/1000;
}
int
Input_data_format_reader_tasklet::
get_current_time() {
  return current_time;
}
int
Input_data_format_reader_tasklet::
get_stop_time() {
  return stop_time;
}
void
Input_data_format_reader_tasklet::
set_stop_time(int64_t ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  SFXC_ASSERT(current_time < us_time);
  stop_time = us_time;
}


void
Input_data_format_reader_tasklet::
push_element() {
  SFXC_ASSERT(input_element_->invalid_bytes_begin >= 0);
  SFXC_ASSERT(input_element_->nr_invalid_bytes >= 0);

  SFXC_ASSERT(input_element_->mark5_data.size() == 
              reader_->size_data_block());
  output_buffer_->push(input_element_);
}

Input_data_format_reader_tasklet::Output_buffer_ptr
Input_data_format_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
}

std::vector< std::vector<int> >
Input_data_format_reader_tasklet::
get_tracks(const Input_node_parameters &input_node_param) {
  return reader_->get_tracks(input_node_param, *input_element_);
}

void
Input_data_format_reader_tasklet::
randomize_block() {
  // Randomize/invalidate the data in the current block

  // Make sure the data has the right size
  size_t size = reader_->size_data_block();
  if (input_element_->mark5_data.size() != size) {
    input_element_->mark5_data.resize(size);
  }

#ifdef SFXC_INVALIDATE_SAMPLES
  input_element_->invalid_bytes_begin = 0;
  input_element_->nr_invalid_bytes = size;

#ifdef SFXC_CHECK_INVALID_SAMPLES
  for (size_t i=0; i<size; i++) {
    input_element_->mark5_data[i] = value_type(0);
  }
#endif // SFXC_CHECK_INVALID_SAMPLES

#else // !SFXC_INVALIDATE_SAMPLES
  for (size_t i=0; i<size; i++) {
    // Randomize data
    // park_miller_random generates 31 random bits
    input_element_->mark5_data[i] = (value_type)park_miller_random();
  }

#endif // SFXC_INVALIDATE_SAMPLES

}
