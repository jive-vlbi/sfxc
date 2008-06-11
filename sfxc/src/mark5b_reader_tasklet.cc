#include "mark5b_reader_tasklet.h"
#include "utils.h"

Mark5b_reader_tasklet::
Mark5b_reader_tasklet(Mark5b_reader_ptr reader,
                      unsigned char *buffer)
    : memory_pool_(10), stop_time(-1) {
  SFXC_ASSERT(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());

  mark5b_reader_ = reader;
  current_time = mark5b_reader_->get_current_time();

  allocate_element();

  memcpy(&input_element_->mark5_data[0],
         buffer,
         SIZE_MK5B_FRAME * SIZE_MK5B_WORD * N_MK5B_BLOCKS_TO_READ);
  input_element_->start_time = current_time;
  input_element_->invalid_bytes_begin = 0;
  input_element_->nr_invalid_bytes = 0;


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

  SFXC_ASSERT(has_work());

  push_element();
  allocate_element();
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
    input_element_->invalid_bytes_begin = 0;
    input_element_->nr_invalid_bytes = 0;
    
    if (!mark5b_reader_->read_new_block(&input_element_->mark5_data[0])) {
#ifdef SFXC_INVALIDATE_SAMPLES
      input_element_->nr_invalid_bytes = 
        SIZE_MK5B_FRAME*SIZE_MK5B_WORD*N_MK5B_BLOCKS_TO_READ;
#else
      randomize_block(0,SIZE_MK5B_FRAME*SIZE_MK5B_WORD*N_MK5B_BLOCKS_TO_READ);
#endif
    }
    current_time = mark5b_reader_->get_current_time();
  }
  input_element_->start_time = current_time;


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
  SFXC_ASSERT(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
  std::vector<value_type> &vector_ = input_element_->mark5_data;
  if (vector_.size() != 
      (N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME*SIZE_MK5B_WORD)) {
    vector_.resize(N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME*SIZE_MK5B_WORD);
  }
  SFXC_ASSERT(input_element_->mark5_data.size() > 0);
}
int
Mark5b_reader_tasklet::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;

  int64_t new_time =
    mark5b_reader_->goto_time((unsigned char *)&input_element_->mark5_data[0],
                              us_time);
  if (new_time != mark5b_reader_->get_current_time()) {
    DEBUG_MSG(new_time << " vs. " << mark5b_reader_->get_current_time());
  }

  // Set the current time to the actual time in the data stream.
  // Might not be the requested time, if no data is available
  current_time = new_time;
  
  input_element_->start_time = current_time;
  input_element_->invalid_bytes_begin = 0;
  input_element_->nr_invalid_bytes = 0;

  if (us_time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << us_time << "us.");
    DEBUG_MSG("Current time is              " << current_time << "us.");
    DEBUG_MSG("Time found is                " << new_time << "us.");
  }

  return current_time/1000;
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

  SFXC_ASSERT(current_time < us_time);
  stop_time = us_time;
}


void
Mark5b_reader_tasklet::
push_element() {
  SFXC_ASSERT(input_element_->invalid_bytes_begin >= 0);
  SFXC_ASSERT(input_element_->nr_invalid_bytes >= 0);

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
        SFXC_ASSERT(bps == 2);
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
