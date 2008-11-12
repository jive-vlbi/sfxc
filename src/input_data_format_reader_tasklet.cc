#include "input_data_format_reader_tasklet.h"

Input_data_format_reader_tasklet::
Input_data_format_reader_tasklet(
  Data_format_reader_ptr reader,
  Data_frame &data)
    : memory_pool_(10), stop_time(-1),
    n_bytes_per_input_word(reader->bytes_per_input_word()) {

  SFXC_ASSERT(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  reader_ = reader;

  assert(data.buffer.size() > 0);

  *input_element_ = data;

  current_time = reader_->get_current_time();
  push_element();

  data_read_=0;
}

Input_data_format_reader_tasklet::~Input_data_format_reader_tasklet(){  }

void Input_data_format_reader_tasklet::stop() {
  /// There is a special associated with the empty interval.
  /// as it will stop the reading thread.
  add_time_interval(0,0);
}

void Input_data_format_reader_tasklet::do_execute() {
  ///DEBUG_MSG(__PRETTY_FUNCTION__ << ":: ENTER");

  /// blocks until we have an interval to process
  fetch_next_time_interval();

  /// then let's work
  while ( !current_interval_.empty() ) {
    /// if there is still some data to process we do it
    if ( current_time < current_interval_.stop_time_ ) do_task();

    /// otherwise we fetch a new interval.
    else fetch_next_time_interval();
  }

  DEBUG_MSG(" INPUT READER WILL EXIT ITS LOOP ");

  /// We close the queue in which we output our data.
  output_buffer_->close();
}

void
Input_data_format_reader_tasklet::
do_task() {
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

  data_read_ += input_element_->buffer.size();

  push_element();
}

void
Input_data_format_reader_tasklet::fetch_next_time_interval() {
  /// Blocking function until a new interval is available
  current_interval_ = intervals_.front_and_pop();

  if ( !current_interval_.empty() ) {
    /// Otherwise the new interval is loaded.
    ///DEBUG_MSG(__PRETTY_FUNCTION__ << ":: SET TIME");
    ///DEBUG_MSG(__PRETTY_FUNCTION__ << ":: val:"<< current_interval_.start_time_ << " cur: "<< current_time);
    current_time = goto_time( current_interval_.start_time_ );
  }
}


void
Input_data_format_reader_tasklet::
add_time_interval(uint64_t us_start_time, uint64_t us_stop_time) {
  ///DEBUG_MSG("Time interval added: " << us_start_time << ":"<< us_stop_time );
  intervals_.push( Time_interval(us_start_time, us_stop_time ) );
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
  input_element_ = memory_pool_.allocate();
}

uint64_t
Input_data_format_reader_tasklet::
goto_time(uint64_t us_time) {
  //int64_t us_time = int64_t(1000)*ms_time;

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

  return current_time;
}

uint64_t
Input_data_format_reader_tasklet::
get_current_time() {
  return current_time;
}

int
Input_data_format_reader_tasklet::
get_stop_time() {
  SFXC_ASSERT(false && "DEPRECATED !");
  return stop_time;
}

void
Input_data_format_reader_tasklet::
set_stop_time(int64_t ms_time) {
  SFXC_ASSERT(false && "DEPRECATED !");
  int64_t us_time = int64_t(1000)*ms_time;

  SFXC_ASSERT(current_time < us_time);
  stop_time = us_time;
}


void
Input_data_format_reader_tasklet::
push_element() {
  SFXC_ASSERT(input_element_->invalid_bytes_begin >= 0);
  SFXC_ASSERT(input_element_->nr_invalid_bytes >= 0);
  SFXC_ASSERT(input_element_->buffer.size() ==
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
  if (input_element_->buffer.size() != size) {
    input_element_->buffer.resize(size);
  }

#ifdef SFXC_INVALIDATE_SAMPLES
  input_element_->invalid_bytes_begin = 0;
  input_element_->nr_invalid_bytes = size;

#ifdef SFXC_CHECK_INVALID_SAMPLES
  for (size_t i=0; i<size; i++) {
    input_element_->buffer[i] = value_type(0);
  }
#endif // SFXC_CHECK_INVALID_SAMPLES

#else // !SFXC_INVALIDATE_SAMPLES
  for (size_t i=0; i<size; i++) {
    // Randomize data
    // park_miller_random generates 31 random bits
    input_element_->buffer[i] = (value_type)park_miller_random();
  }

#endif // SFXC_INVALIDATE_SAMPLES
}
