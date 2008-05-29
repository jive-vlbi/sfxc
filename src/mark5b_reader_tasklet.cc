#include "mark5b_reader_tasklet.h"
#include "utils.h"

Mark5b_reader_tasklet::
Mark5b_reader_tasklet(Mark5b_reader_ptr reader)
    : memory_pool_(10), stop_time(-1) {
  assert(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());

  mark5b_reader_ = reader;
  current_time = mark5b_reader_->get_current_time();
  allocate_element();
  mark5b_reader_->read_new_block(&input_element_->mk4_data[0]);
  
  input_element_->start_time = current_time;


#ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream chexid;
  std::stringstream monid;

  inputid << "inputnode" << RANK_OF_NODE;
  chexid << inputid.str() << "_mark5breader";
  monid << chexid.str() << "_monitor_speed";

  monitor_.init(monid.str(), 1000, "stats/");
  monitor_.add_property(inputid.str(), "is_a", "inputnode");
  monitor_.add_property(inputid.str(), "has", chexid.str() );
  monitor_.add_property(chexid.str(), "is_a", "mark5breader");
  monitor_.add_property(chexid.str(), "has", monid.str() );

#endif //RUNTIME_STATISTIC
}
void
Mark5b_reader_tasklet::
do_task() {
#ifdef RUNTIME_STATISTIC
  monitor_.begin_measure();
#endif // RUNTIME_STATISTIC

  assert(has_work());

  push_element();
  allocate_element();
  input_element_->start_time = current_time;

  if (mark5b_reader_->eof()) {
#ifdef SFXC_INVALIDATE_SAMPLES
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
#else
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = 0;
    randomize_block(0,SIZE_MK5B_FRAME*SIZE_MK5B_WORD);
#endif

    current_time += mark5b_reader_->time_between_headers();
  } else {
    if (!mark5b_reader_->read_new_block(&input_element_->mk4_data[0])) {
#ifdef SFXC_INVALIDATE_SAMPLES
      input_element_->invalid_bytes_begin = 0;
      input_element_->nr_invalid_bytes = SIZE_MK5B_FRAME*SIZE_MK5B_WORD;
#else
      input_element_->invalid_bytes_begin = 0;
      input_element_->nr_invalid_bytes = 0;
      randomize_block(0,SIZE_MK5B_FRAME*SIZE_MK5B_WORD);
#endif
    }
    current_time = mark5b_reader_->get_current_time();
  }

#ifdef RUNTIME_STATISTIC
  monitor_.end_measure(SIZE_MK5B_FRAME*SIZE_MK5B_WORD);
#endif // RUNTIME_STATISTIC
}

bool
Mark5b_reader_tasklet::
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
Mark5b_reader_tasklet::
allocate_element() {
  assert(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
  std::vector<value_type> &vector_ = input_element_->mk4_data;
  if (vector_.size() != 
      (N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME*SIZE_MK5B_WORD)) {
    vector_.resize(N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME*SIZE_MK5B_WORD);
  }
  assert(input_element_->mk4_data.size() > 0);
}
int
Mark5b_reader_tasklet::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  int64_t new_time =
    mark5b_reader_->goto_time((unsigned char *)&input_element_->mk4_data[0],
                             us_time);
  current_time = mark5b_reader_->get_current_time();
  input_element_->start_time = current_time;

  if (us_time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << us_time/1000
              << "ms not found. Current time is " << new_time);
  }
  return new_time/1000;
}
int
Mark5b_reader_tasklet::
get_current_time() {
  return current_time;
}
int
Mark5b_reader_tasklet::
get_stop_time() {
  return stop_time;
}
void
Mark5b_reader_tasklet::
set_stop_time(int64_t ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  assert(current_time < us_time);
  stop_time = us_time;
}


void
Mark5b_reader_tasklet::
push_element() {
  // Mark the mark5b header as invalid , if no invalid sequence is set yet
  assert(input_element_->mk4_data.size() > 0);
  if (input_element_->invalid_bytes_begin < 0) {
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = 0;
  }
  output_buffer_->push(input_element_);
}

Mark5b_reader_tasklet::Output_buffer_ptr
Mark5b_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
}

std::vector< std::vector<int> >
Mark5b_reader_tasklet::
get_tracks(const Input_node_parameters &input_node_param) {
  std::vector< std::vector<int> > result;
  result.resize(input_node_param.channels.size());
  for (size_t i=0; i<input_node_param.channels.size(); i++) {
    int bps = input_node_param.channels[i].bits_per_sample();
    int fo  = input_node_param.channels[i].sign_tracks.size();
    result[i].resize(bps * fo);
    for (size_t j=0; j<input_node_param.channels[i].sign_tracks.size(); j++) {
      if (bps == 1) {
        result[i][j] = input_node_param.channels[i].sign_tracks[j];
      } else {
        assert(bps == 2);
        result[i][2*j] = input_node_param.channels[i].sign_tracks[j];
        result[i][2*j+1] = input_node_param.channels[i].magn_tracks[j];
      }
    }
  }
  return result;
}

void 
Mark5b_reader_tasklet::
set_parameters(const Input_node_parameters &input_param) {
  mark5b_reader_->set_track_bit_rate(input_param.track_bit_rate);
}
