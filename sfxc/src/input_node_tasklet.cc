/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id:$
 *
 */

#include "input_node_tasklet.h"
#include "utils.h"
#include "mark5a_reader.h"
#include "monitor.h"

Input_node_tasklet *
get_input_node_tasklet_mark5a(boost::shared_ptr<Data_reader> reader) {

  // Maximal buffer size
  unsigned char buffer[SIZE_MK5A_FRAME*sizeof(uint64_t)];

  boost::shared_ptr<Mark5a_reader> mark5a_reader_ptr = 
    boost::shared_ptr<Mark5a_reader>(get_mark5a_reader(reader, buffer));

  return new Input_node_tasklet(mark5a_reader_ptr, buffer);
}


Input_node_tasklet *
get_input_node_tasklet_mark5b(boost::shared_ptr<Data_reader> reader) {
  unsigned char buffer[(SIZE_MK5B_FRAME+SIZE_MK5B_HEADER) *
                       SIZE_MK5B_WORD *
                       N_MK5B_BLOCKS_TO_READ];


  boost::shared_ptr<Mark5b_reader> mark5b_reader_ptr = 
    boost::shared_ptr<Mark5b_reader>(new Mark5b_reader(reader, buffer));

  return new Input_node_tasklet(mark5b_reader_ptr, buffer);
}

Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type) {
  assert(type != UNINITIALISED);

  if (type == MARK5A) {
    return get_input_node_tasklet_mark5a(reader);
  }
  if (type == MARK5B) {
    return get_input_node_tasklet_mark5b(reader);
  }
  return NULL;
}

void 
Input_node_tasklet::add_time_interval(int32_t start_time, int32_t stop_time) {
  if (time_intervals.empty()) {
    // Check whether we are still processing
    if ((get_current_time() == get_stop_time()) || 
        (get_stop_time() < 0)) {
      set_time_interval(start_time, stop_time);
    } else {
      time_intervals.push(std::make_pair(start_time, stop_time));
    }
  } else {
    time_intervals.push(std::make_pair(start_time, stop_time));
  }
}

Input_node_tasklet::
Input_node_tasklet(Mark5a_reader_ptr_ mark5a_reader_ptr,
                   unsigned char buffer[])
    : mark5a_reader_(NULL),
    mark5b_reader_(NULL),
    channel_extractor_(SIZE_MK5A_FRAME, mark5a_reader_ptr->N),
    did_work(true),
    n_bytes_per_input_word(mark5a_reader_ptr->N),
    transport_type(MARK5A) {

  mark5a_reader_ = new Mark5a_reader_tasklet(mark5a_reader_ptr, buffer),

  channel_extractor_.connect_to(mark5a_reader_->get_output_buffer());

  initialise();
}

Input_node_tasklet::
Input_node_tasklet(Mark5b_reader_ptr_ mark5b_reader_ptr,
                   unsigned char buffer[])
    : mark5a_reader_(NULL),
    mark5b_reader_(NULL),
    channel_extractor_(N_MK5B_BLOCKS_TO_READ*SIZE_MK5B_FRAME, sizeof(int32_t)),
    did_work(true),
    n_bytes_per_input_word(sizeof(int32_t)),
    transport_type(MARK5B) {

  mark5b_reader_ = new Mark5b_reader_tasklet(mark5b_reader_ptr, buffer),

  channel_extractor_.connect_to(mark5b_reader_->get_output_buffer());

  initialise();
}

void Input_node_tasklet::initialise() {
#ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream compid;
  std::stringstream monid;
  std::stringstream tt;

  inputid << "inputnode" << RANK_OF_NODE;

  compid << inputid.str() << "_dotask";
  monid << compid.str() << "_monitor_state";
  dotask_state_.init(monid.str());
  dotask_state_.add_property(inputid.str(), "is_a", "inputnode");
  dotask_state_.add_property(inputid.str(), "has", compid.str() );
  dotask_state_.add_property(compid.str(), "is_a", "inputnode_dotaskloop");
  dotask_state_.add_property(compid.str(), "has", monid.str() );
  tt.str(monid.str());

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_mark5a_reader";
  monid << compid.str() << "_monitor_state";
  mark5a_reader_state_.init(monid.str());
  mark5a_reader_state_.add_property(inputid.str(), "is_a", "inputnode");
  mark5a_reader_state_.add_property(inputid.str(), "has", compid.str() );
  mark5a_reader_state_.add_property(compid.str(), "is_a", "mark5a_reader");
  mark5a_reader_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );


  compid.str("");
  monid.str("");
  compid << inputid.str() << "_channelextractor";
  monid << compid.str() << "_monitor_state";
  chex_state_.init(monid.str());
  chex_state_.add_property(inputid.str(), "is_a", "inputnode");
  chex_state_.add_property(inputid.str(), "has", compid.str() );
  chex_state_.add_property(compid.str(), "is_a", "channel_extractor");
  chex_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );


  compid.str("");
  monid.str("");
  compid << inputid.str() << "_integerdelaycorr";
  monid << compid.str() << "_monitor_state";
  integerdelay_state_.init(monid.str());
  integerdelay_state_.add_property(inputid.str(), "is_a", "inputnode");
  integerdelay_state_.add_property(inputid.str(), "has", compid.str() );
  integerdelay_state_.add_property(compid.str(), "is_a", "integer_delay_correction");
  integerdelay_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );


  compid.str("");
  monid.str("");
  compid << inputid.str() << "_outputwriter";
  monid << compid.str() << "_monitor_state";
  outputwriter_state_.init(monid.str());
  outputwriter_state_.add_property(inputid.str(), "is_a", "inputnode");
  outputwriter_state_.add_property(inputid.str(), "has", compid.str() );
  outputwriter_state_.add_property(compid.str(), "is_a", "output_writers");
  outputwriter_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );




#endif //RUNTIME_STATISTIC


}

Input_node_tasklet::~Input_node_tasklet() {
#if PRINT_TIMER
  PROGRESS_MSG("Time mar4_reader:       " << mark5a_reader_timer_.measured_time());
  PROGRESS_MSG("Time integer_delay:     " << integer_delay_timer_.measured_time());
  PROGRESS_MSG("Time channel_extractor: " << channel_extractor_timer_.measured_time());
  PROGRESS_MSG("Time data_writers:      " << data_writers_timer_.measured_time());
#endif
}



void
Input_node_tasklet::
do_task() {
  did_work = false;

  RT_STAT( dotask_state_.begin_measure() );

  mark5a_reader_timer_.resume();
  if (transport_type == MARK5A) {
    assert(mark5a_reader_ != NULL);
    if (mark5a_reader_->has_work()) {
      
      RT_STAT( mark5a_reader_state_.begin_measure() );
      assert(mark5a_reader_ != NULL);
      mark5a_reader_->do_task();
      RT_STAT(mark5a_reader_state_.end_measure(1) );
      
      did_work = true;
    } else {
      // Check whether we can go to a new time interval
      if (get_current_time() == get_stop_time()) {
        if (!time_intervals.empty()) {
          set_time_interval(time_intervals.front().first,
                            time_intervals.front().second);
          time_intervals.pop();
          did_work = true;
        }
      }
    }
  } else if (transport_type == MARK5B) {
    assert(mark5b_reader_ != NULL);
    if (mark5b_reader_->has_work()) {
      
      RT_STAT( mark5breader_state_.begin_measure() );
      assert(mark5b_reader_ != NULL);
      mark5b_reader_->do_task();
      RT_STAT(mark5breader_state_.end_measure(1) );
      
      did_work = true;
    } else {
      // Check whether we can go to a new time interval
      if (get_current_time() == get_stop_time()) {
        if (!time_intervals.empty()) {
          set_time_interval(time_intervals.front().first,
                            time_intervals.front().second);
          time_intervals.pop();
          did_work = true;
        }
      }
    }
  } else {
    assert(false);
  }
  mark5a_reader_timer_.stop();


  channel_extractor_timer_.resume();
  if (channel_extractor_.has_work()) {
    RT_STAT(chex_state_.begin_measure());
    channel_extractor_.do_task();
    RT_STAT(chex_state_.end_measure(1));
    did_work = true;
  }
  channel_extractor_timer_.stop();

  integer_delay_timer_.resume();
  RT_STAT(integerdelay_state_.begin_measure() );
  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    while (integer_delay_[i]->has_work()) {

      integer_delay_[i]->do_task();

      did_work = true;
    }
  }
  RT_STAT(integerdelay_state_.end_measure(1));
  integer_delay_timer_.stop();


  data_writers_timer_.resume();
  RT_STAT( outputwriter_state_.begin_measure() );
  for (size_t i=0; i<data_writers_.size(); i++) {
    while (data_writers_[i].has_work()) {
      data_writers_[i].do_task();
      did_work = true;
    }
  }
  RT_STAT( outputwriter_state_.end_measure(1) );
  data_writers_timer_.stop();

  RT_STAT( dotask_state_.end_measure(1) );
}

bool
Input_node_tasklet::
has_work() {
  return did_work;
}

void
Input_node_tasklet::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    integer_delay_[i]->set_delay_table(table);
  }

  did_work = true;
}

void
Input_node_tasklet::
set_parameters(const Input_node_parameters &input_node_param,
               int node_nr) {
  if (transport_type == MARK5A) {
    mark5a_reader_->set_parameters(input_node_param);
    channel_extractor_.set_parameters(input_node_param,
                                      mark5a_reader_->get_tracks(input_node_param));
  } else if (transport_type == MARK5B) {
    mark5b_reader_->set_parameters(input_node_param);
    channel_extractor_.set_parameters(input_node_param,
                                      mark5b_reader_->get_tracks(input_node_param));
  } else {
    assert(false);
  }

  size_t number_frequency_channels = input_node_param.channels.size();
  integer_delay_.resize(number_frequency_channels, NULL);
  data_writers_.resize(number_frequency_channels);

  for (size_t i=0; i < number_frequency_channels; i++) {
    if (integer_delay_[i] == NULL) {
      integer_delay_[i] = new Integer_delay_tasklet_();
      if (delay_table.initialised()) {
        integer_delay_[i]->set_delay_table(delay_table);
      }
    }
    integer_delay_[i]->connect_to(channel_extractor_.get_output_buffer(i));
    integer_delay_[i]->set_parameters(input_node_param, node_nr);

    data_writers_[i].connect_to(integer_delay_[i]->get_output_buffer());
    data_writers_[i].set_parameters(input_node_param);
  }

  did_work = true;
}

void
Input_node_tasklet::
set_time_interval(int32_t start_time, int32_t stop_time) {
  int new_time;
  assert(!integer_delay_.empty());
  assert(integer_delay_[0] != NULL);
  if (transport_type == MARK5A) {
    assert(mark5a_reader_ != NULL);
    new_time = mark5a_reader_->goto_time(start_time);
    mark5a_reader_->set_stop_time(stop_time);
  } else {
    assert(transport_type == MARK5B);
    assert(mark5b_reader_ != NULL);
    new_time = mark5b_reader_->goto_time(start_time);
    mark5b_reader_->set_stop_time(stop_time);
  }

  for (size_t i=0; i < integer_delay_.size(); i++) {
    integer_delay_[i]->set_time(int64_t(1000)*new_time);
  }

  did_work = true;
}
int
Input_node_tasklet::
get_current_time() {
  if (transport_type == MARK5A) {
    assert(mark5a_reader_ != NULL);
    return mark5a_reader_->get_current_time();
  } else {
    assert(mark5b_reader_ != NULL);
    return mark5b_reader_->get_current_time();
  }
}
int
Input_node_tasklet::
get_stop_time() {
  if (transport_type == MARK5A) {
    assert(mark5a_reader_ != NULL);
    return mark5a_reader_->get_stop_time();
  } else {
    assert(mark5b_reader_ != NULL);
    return mark5b_reader_->get_stop_time();
  }
}

void
Input_node_tasklet::
add_data_writer(size_t i,
                Data_writer_ptr_ data_writer,
                int nr_seconds) {
  did_work = true;
  assert(i < data_writers_.size());
  assert(!integer_delay_.empty());
  assert(integer_delay_[i] != NULL);
  int size_slice = integer_delay_[i]->bytes_of_output(nr_seconds);
  data_writers_[i].add_data_writer(data_writer, size_slice);
}
