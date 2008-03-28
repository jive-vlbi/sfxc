/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INPUT_NODE_TASKLET_IMPL_H
#define INPUT_NODE_TASKLET_IMPL_H

#include <memory_pool.h>
#include "timer.h"

#include "input_node_tasklet.h"
#include "input_node_types.h"
#include "utils.h"

#include "mark4_reader_tasklet.h"
#include "integer_delay_correction_per_channel.h"
#include "void_consuming_tasklet.h"
#include "channel_extractor_tasklet.h"
#include "input_node_data_writer_tasklet.h"

#include "monitor.h"



template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet<Type>                 Mark4_reader_tasklet_;
  typedef Channel_extractor_tasklet                  Channel_extractor_tasklet_;
  typedef Integer_delay_correction_per_channel<Type> Integer_delay_tasklet_;
  typedef Input_node_data_writer_tasklet<Type>       Data_writer_tasklet_;

  Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader, char *buffer);

  ~Input_node_tasklet_implementation() {

#if PRINT_TIMER
    DEBUG_MSG("Time mar4_reader:       " << mark4_reader_timer_.measured_time());
    DEBUG_MSG("Time integer_delay:     " << integer_delay_timer_.measured_time());
    DEBUG_MSG("Time channel_extractor: " << channel_extractor_timer_.measured_time());
    DEBUG_MSG("Time data_writers:      " << data_writers_timer_.measured_time());
#endif

  }

  void do_task();
  bool has_work();

  const char *name() {
    return __PRETTY_FUNCTION__;
  }


  // Inherited from Input_node_tasklet
  void set_delay_table(Delay_table_akima &delay);
  void set_parameters(const Input_node_parameters &input_node_param,
                      int node_nr);
  int goto_time(int time);
  int get_current_time();
  void set_stop_time(int time);
  //  bool append_time_slice(const Time_slice &time_slice);
  void add_data_writer(size_t i,
                       Data_writer_ptr_ data_writer,
                       int nr_seconds);

private:
  //  std::list<Time_slice>                time_slices_;
  Mark4_reader_tasklet_                mark4_reader_;
  Channel_extractor_tasklet_           channel_extractor_;

  // Pointer because we can not copy construct the Integer_delay_tasklet_
  // because of the memory pool
  std::vector<Integer_delay_tasklet_ *>  integer_delay_;
  std::vector<Data_writer_tasklet_>    data_writers_;

  bool did_work;

  Timer mark4_reader_timer_, integer_delay_timer_, channel_extractor_timer_, data_writers_timer_;

  Delay_table_akima delay_table;
  
  #ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed mark4reader_state_;
  QOS_MonitorSpeed chex_state_;
  QOS_MonitorSpeed integerdelay_state_;
  QOS_MonitorSpeed outputwriter_state_;
  QOS_MonitorSpeed dotask_state_;
  #endif //RUNTIME_STATISTIC
};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader,
                                  char *buffer)
    : mark4_reader_(reader, buffer),
    channel_extractor_(mark4_reader_.size_input_word()),
did_work(true) {
  channel_extractor_.connect_to(mark4_reader_.get_output_buffer());
  
  #ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream compid;
  std::stringstream monid;
  std::stringstream tt;
  
  inputid << "inputnode" << RANK_OF_NODE;
 
  compid.str(""); monid.str("");
  compid << inputid.str() << "_dotask";
  monid << compid.str() << "_monitor_state";  
  dotask_state_.init(monid.str());
  dotask_state_.add_property(inputid.str(), "is_a", "inputnode");
  dotask_state_.add_property(inputid.str(), "has", compid.str() );
  dotask_state_.add_property(compid.str(), "is_a", "inputnode_dotaskloop");
  dotask_state_.add_property(compid.str(), "has", monid.str() );
  tt.str(monid.str());
  
  compid << inputid.str() << "_mark4reader";
  monid << compid.str() << "_monitor_state";
  mark4reader_state_.init(monid.str());
  mark4reader_state_.add_property(inputid.str(), "is_a", "inputnode");
  mark4reader_state_.add_property(inputid.str(), "has", compid.str() );
  mark4reader_state_.add_property(compid.str(), "is_a", "mark4_reader");
  mark4reader_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );
  
  
  compid.str(""); monid.str("");
  compid << inputid.str() << "_channelextractor";
  monid << compid.str() << "_monitor_state";
  chex_state_.init(monid.str());
  chex_state_.add_property(inputid.str(), "is_a", "inputnode");
  chex_state_.add_property(inputid.str(), "has", compid.str() );
  chex_state_.add_property(compid.str(), "is_a", "channel_extractor");
  chex_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );
 

  compid.str(""); monid.str("");
  compid << inputid.str() << "_integerdelaycorr";
  monid << compid.str() << "_monitor_state";  
  integerdelay_state_.init(monid.str());
  integerdelay_state_.add_property(inputid.str(), "is_a", "inputnode");
  integerdelay_state_.add_property(inputid.str(), "has", compid.str() );
  integerdelay_state_.add_property(compid.str(), "is_a", "integer_delay_correction");
  integerdelay_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );
 

  compid.str(""); monid.str("");
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


template <class Type>
void
Input_node_tasklet_implementation<Type>::
do_task() {
  did_work = false;
  
  RT_STAT( dotask_state_.begin_measure() );
  

  mark4_reader_timer_.resume();
  if (mark4_reader_.has_work()){
  
    RT_STAT( mark4reader_state_.begin_measure() );
    mark4_reader_.do_task();
    RT_STAT(mark4reader_state_.end_measure(1) );

    did_work = true;
  }
  mark4_reader_timer_.stop();
  
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

template <class Type>
bool
Input_node_tasklet_implementation<Type>::
has_work() {
  return did_work;
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    integer_delay_[i]->set_delay_table(table);
  }

  did_work = true;
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_parameters(const Input_node_parameters &input_node_param,
               int node_nr) {
  channel_extractor_.set_parameters(input_node_param,
                                    mark4_reader_.get_tracks(input_node_param));

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

template <class Type>
int
Input_node_tasklet_implementation<Type>::
goto_time(int time) {
  assert(!integer_delay_.empty());
  assert(integer_delay_[0] != NULL);
  int new_time = mark4_reader_.goto_time(time);

  for (size_t i=0; i < integer_delay_.size(); i++) {
    integer_delay_[i]->set_time(int64_t(1000)*new_time);
  }

  did_work = true;
  return new_time;
}
template <class Type>
int
Input_node_tasklet_implementation<Type>::
get_current_time() {
  return mark4_reader_.get_current_time();
}
template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_stop_time(int time) {
  did_work = true;

  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    integer_delay_[i]->set_stop_time(int64_t(1000)*time);
  }
  return mark4_reader_.set_stop_time(int64_t(1000)*time);
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
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

#endif // INPUT_NODE_TASKLET_IMPL_H
