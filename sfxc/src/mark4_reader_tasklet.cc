#include "mark4_reader_tasklet.h"

Mark4_reader_tasklet::
Mark4_reader_tasklet(boost::shared_ptr<Data_reader> reader,
                     char *buffer,
                     int n_bytes_per_input_word)
    : memory_pool_(10), stop_time(-1),
    n_bytes_per_input_word(n_bytes_per_input_word) {

  assert(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  mark4_reader_ =
    boost::shared_ptr<Mark4_reader >(new Mark4_reader(reader,
                                     n_bytes_per_input_word,
                                     (unsigned char *)buffer,
                                     (unsigned char *)&input_element_.data().mk4_data[0]));
  current_time = mark4_reader_->get_current_time();
  input_element_.data().start_time = current_time;


#ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream chexid;
  std::stringstream monid;

  inputid << "inputnode" << RANK_OF_NODE;
  chexid << inputid.str() << "_mark4reader";
  monid << chexid.str() << "_monitor_speed";

  monitor_.init(monid.str(), 1000, "stats/");
  monitor_.add_property(inputid.str(), "is_a", "inputnode");
  monitor_.add_property(inputid.str(), "has", chexid.str() );
  monitor_.add_property(chexid.str(), "is_a", "mark4reader");
  monitor_.add_property(chexid.str(), "has", monid.str() );

#endif //RUNTIME_STATISTIC
}
void
Mark4_reader_tasklet::
do_task() {
#ifdef RUNTIME_STATISTIC
  monitor_.begin_measure();
#endif // RUNTIME_STATISTIC

  assert(has_work());

  push_element();
  allocate_element();

  if (mark4_reader_->eof()) {
    randomize_block(0,SIZE_MK4_FRAME*n_bytes_per_input_word);
    current_time += mark4_reader_->time_between_headers();
  } else {
    if (!mark4_reader_->read_new_block(&input_element_.data().mk4_data[0])) {
      randomize_block(0,SIZE_MK4_FRAME*n_bytes_per_input_word);
    }
    current_time = mark4_reader_->get_current_time();
  }
  input_element_.data().start_time = current_time;

#ifdef RUNTIME_STATISTIC
  monitor_.end_measure(SIZE_MK4_FRAME*n_bytes_per_input_word);
#endif // RUNTIME_STATISTIC
}

bool
Mark4_reader_tasklet::
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
Mark4_reader_tasklet::
allocate_element() {
  assert(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
  std::vector<value_type> &vector_ = input_element_.data().mk4_data;
  if (vector_.size() != (SIZE_MK4_FRAME*n_bytes_per_input_word)) {
    vector_.resize(SIZE_MK4_FRAME*n_bytes_per_input_word);
  }
}
int
Mark4_reader_tasklet::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  int64_t new_time =
    mark4_reader_->goto_time((unsigned char *)&input_element_.data().mk4_data[0],
                             us_time);
  current_time = mark4_reader_->get_current_time();
  input_element_.data().start_time = current_time;

  if (us_time != new_time) {
    DEBUG_MSG("New time " << us_time
              << "us not found. Current time is " << new_time);
  }
  return new_time/1000;
}
int
Mark4_reader_tasklet::
get_current_time() {
  return current_time;
}
void
Mark4_reader_tasklet::
set_stop_time(int64_t time) {
  assert(current_time < time);
  stop_time = time;
}


void
Mark4_reader_tasklet::
push_element() {
  randomize_block(0, SIZE_MK4_HEADER*n_bytes_per_input_word);
  output_buffer_->push(input_element_);
}
Mark4_reader_tasklet::Output_buffer_ptr
Mark4_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
}
std::vector< std::vector<int> >
Mark4_reader_tasklet::
get_tracks(const Input_node_parameters &input_node_param) {
  return mark4_reader_->get_tracks(input_node_param,
                                   (unsigned char *)&input_element_.data().mk4_data[0]);
}
void
Mark4_reader_tasklet::
randomize_block(int start, int stop) {
  // Randomize header
  for (size_t i=0; i<SIZE_MK4_HEADER*n_bytes_per_input_word; i++) {
#ifdef SFXC_DETERMINISTIC
    input_element_.data().mk4_data[i] = value_type(0);
#else
    // Randomize data
    // park_miller_random generates 31 random bits
    input_element_.data().mk4_data[i] = (value_type)park_miller_random();
#endif
  }
}
