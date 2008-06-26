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
#include "mark5b_reader.h"
#include "monitor.h"

Input_node_tasklet *
get_input_node_tasklet_mark5a(boost::shared_ptr<Data_reader> reader) {

  // Maximal buffer size
  Input_data_format_reader::Data_frame data;

  boost::shared_ptr<Mark5a_reader> mark5a_reader_ptr =
    boost::shared_ptr<Mark5a_reader>( get_mark5a_reader(reader, data) );

  return new Input_node_tasklet(mark5a_reader_ptr, data);
}


Input_node_tasklet *
get_input_node_tasklet_mark5b(boost::shared_ptr<Data_reader> reader) {
  typedef boost::shared_ptr<Input_data_format_reader> Input_reader_ptr;

  Input_data_format_reader::Data_frame   data;

  Input_reader_ptr   mark5b_reader_ptr(new Mark5b_reader(reader, data));

  return new Input_node_tasklet(mark5b_reader_ptr, data);
}

Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type) {
  SFXC_ASSERT(type != UNINITIALISED);

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
Input_node_tasklet(Input_reader_ptr_ reader_ptr,
                   Input_reader_::Data_frame &data)
  : reader_(reader_ptr, data),
    channel_extractor_(reader_ptr->size_data_block() /
                       reader_ptr->bytes_per_input_word(), 
                       reader_ptr->bytes_per_input_word()),
    did_work(true),
    n_bytes_per_input_word(reader_ptr->bytes_per_input_word()) {

  channel_extractor_.connect_to(reader_.get_output_buffer());

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
  compid << inputid.str() << "_reader";
  monid << compid.str() << "_monitor_state";
  reader_state_.init(monid.str());
  reader_state_.add_property(inputid.str(), "is_a", "inputnode");
  reader_state_.add_property(inputid.str(), "has", compid.str() );
  reader_state_.add_property(compid.str(), "is_a", "reader");
  reader_state_.add_property(compid.str(), "has", monid.str() );
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
  channel_extractor_.empty_input_queue();

  for (size_t i = 0; i < integer_delay_.size(); i++) {
    integer_delay_[i]->empty_input_queue();
  }

  for (size_t i = 0; i < data_writers_.size(); i++) {
    data_writers_[i].empty_input_queue();
  }

#if PRINT_TIMER
  PROGRESS_MSG("Time mar4_reader:       " << reader_timer_.measured_time());
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

  reader_timer_.resume();
  if (reader_.has_work()) {

    RT_STAT( reader_state_.begin_measure() );
    reader_.do_task();
    RT_STAT(reader_state_.end_measure(1) );

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
  reader_timer_.stop();


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
    SFXC_ASSERT(integer_delay_[i] != NULL);
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
    SFXC_ASSERT(integer_delay_[i] != NULL);
    integer_delay_[i]->set_delay_table(table);
  }

  did_work = true;
}

void
Input_node_tasklet::
set_parameters(const Input_node_parameters &input_node_param,
               int node_nr) {
  reader_.set_parameters(input_node_param);
  channel_extractor_.set_parameters(input_node_param,
                                    reader_.get_tracks(input_node_param));

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
  SFXC_ASSERT(!integer_delay_.empty());
  SFXC_ASSERT(integer_delay_[0] != NULL);

  reader_.goto_time(start_time);
  reader_.set_stop_time(stop_time);

  for (size_t i=0; i < integer_delay_.size(); i++) {
    integer_delay_[i]->set_time(int64_t(1000)*start_time);
    integer_delay_[i]->set_stop_time(int64_t(1000)*stop_time);
  }

  did_work = true;
}
int
Input_node_tasklet::
get_current_time() {
  return reader_.get_current_time();
}
int
Input_node_tasklet::
get_stop_time() {
  return reader_.get_stop_time();
}

void
Input_node_tasklet::
add_data_writer(size_t i,
                Data_writer_ptr_ data_writer) {
  did_work = true;
  SFXC_ASSERT(i < data_writers_.size());
  SFXC_ASSERT(!integer_delay_.empty());
  SFXC_ASSERT(integer_delay_[i] != NULL);
  // Number of bytes for one integration slice
  int size_slice = integer_delay_[i]->bytes_of_output();
  data_writers_[i].add_data_writer(data_writer, size_slice);
}
