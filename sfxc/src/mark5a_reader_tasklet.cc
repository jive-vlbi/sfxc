#include "mark5a_reader_tasklet.h"

Mark5a_reader_tasklet::
Mark5a_reader_tasklet(Mark5a_reader_ptr reader,
                     unsigned char buffer[])
    : memory_pool_(10), stop_time(-1),
    n_bytes_per_input_word(reader->N) {

  assert(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  mark5a_reader_ = reader;

  assert(&input_element_->mark5_data[0] != NULL);
  // Copy the first data block
  memcpy((unsigned char *)&input_element_->mark5_data[0],
         (unsigned char *)buffer,
         SIZE_MK5A_FRAME*n_bytes_per_input_word);

  current_time = mark5a_reader_->get_current_time();
  input_element_->start_time = current_time;


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
Mark5a_reader_tasklet::
do_task() {
#ifdef RUNTIME_STATISTIC
  monitor_.begin_measure();
#endif // RUNTIME_STATISTIC

  assert(has_work());

  push_element();
  allocate_element();

  if (mark5a_reader_->eof()) {
#ifdef SFXC_INVALIDATE_SAMPLES
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = SIZE_MK5A_FRAME*n_bytes_per_input_word;
#else
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = 0;
    randomize_block(0,SIZE_MK5A_FRAME*n_bytes_per_input_word);
#endif

    current_time += mark5a_reader_->time_between_headers();
  } else if (current_time < mark5a_reader_->get_current_time()) {
    // We don't have data yet
#ifdef SFXC_INVALIDATE_SAMPLES
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = SIZE_MK5A_FRAME*n_bytes_per_input_word;
#else
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = 0;
    randomize_block(0,SIZE_MK5A_FRAME*n_bytes_per_input_word);
#endif

    current_time += mark5a_reader_->time_between_headers();
  } else {
    if (!mark5a_reader_->read_new_block(&input_element_->mark5_data[0])) {
#ifdef SFXC_INVALIDATE_SAMPLES
      input_element_->invalid_bytes_begin = 0;
      input_element_->nr_invalid_bytes = SIZE_MK5A_FRAME*n_bytes_per_input_word;
#else
      input_element_->invalid_bytes_begin = 0;
      input_element_->nr_invalid_bytes = 0;
      randomize_block(0,SIZE_MK5A_FRAME*n_bytes_per_input_word);
#endif
    }
    current_time = mark5a_reader_->get_current_time();
  }
  input_element_->start_time = current_time;

#ifdef RUNTIME_STATISTIC
  monitor_.end_measure(SIZE_MK5A_FRAME*n_bytes_per_input_word);
#endif // RUNTIME_STATISTIC
}

bool
Mark5a_reader_tasklet::
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
Mark5a_reader_tasklet::
allocate_element() {
  assert(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
  std::vector<value_type> &vector_ = input_element_->mark5_data;
  if (vector_.size() != (SIZE_MK5A_FRAME*n_bytes_per_input_word)) {
    vector_.resize(SIZE_MK5A_FRAME*n_bytes_per_input_word);
  }
}
int
Mark5a_reader_tasklet::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  int64_t new_time =
    mark5a_reader_->goto_time((unsigned char *)&input_element_->mark5_data[0],
                              us_time);
  assert(new_time == mark5a_reader_->get_current_time());

  if (us_time < new_time) {
    // Going to a time before the data started, start sending invalid data
    current_time = us_time;
  } else {
    // Normal case: 
    current_time = new_time;
  }
  
  input_element_->start_time = current_time;

  if (us_time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << us_time << "us.");
    DEBUG_MSG("Current time is              " << current_time << "us.");
    DEBUG_MSG("Time found is                " << new_time << "us.");
  }

  return current_time/1000;
}
int
Mark5a_reader_tasklet::
get_current_time() {
  return current_time;
}
int
Mark5a_reader_tasklet::
get_stop_time() {
  return stop_time;
}
void
Mark5a_reader_tasklet::
set_stop_time(int64_t ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  assert(current_time < us_time);
  stop_time = us_time;
}


void
Mark5a_reader_tasklet::
push_element() {
  // Mark the mark5a header as invalid , if no invalid sequence is set yet
#ifdef SFXC_INVALIDATE_SAMPLES
  if (input_element_->invalid_bytes_begin < 0) {
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = SIZE_MK5A_HEADER*n_bytes_per_input_word;
  }
#  ifdef SFXC_CHECK_INVALID_SAMPLES
  for (size_t i=0; i<SIZE_MK5A_HEADER*n_bytes_per_input_word; i++) {
    input_element_.data().mark5_data[i] = value_type(INVALID_PATTERN);
  }
#  endif
#else
  input_element_->invalid_bytes_begin = 0;
  input_element_->nr_invalid_bytes = 0;
  randomize_block(0, SIZE_MK5A_HEADER*n_bytes_per_input_word);
#endif
  output_buffer_->push(input_element_);
}

Mark5a_reader_tasklet::Output_buffer_ptr
Mark5a_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
}

std::vector< std::vector<int> >
Mark5a_reader_tasklet::
get_tracks(const Input_node_parameters &input_node_param) {
  return mark5a_reader_->get_tracks(input_node_param,
                                   (unsigned char *)&input_element_->mark5_data[0]);
}

void
Mark5a_reader_tasklet::
randomize_block(int start, int stop) {
  // Randomize header
  for (size_t i=0; i<SIZE_MK5A_HEADER*n_bytes_per_input_word; i++) {
#ifdef SFXC_INVALIDATE_SAMPLES
#ifdef SFXC_CHECK_INVALID_SAMPLES
    input_element_.data().mark5_data[i] = value_type(0);
#endif
#else
    // Randomize data
    // park_miller_random generates 31 random bits
    input_element_.data().mark5_data[i] = (value_type)park_miller_random();
#endif
  }
}
