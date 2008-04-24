#include "input_node_tasklet_impl.h"

Input_node_tasklet_implementation::
Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader,
                                  char *buffer,
                                  size_t n_bytes_per_input_word_)
    : mark4_reader_(reader, buffer, n_bytes_per_input_word_),
    channel_extractor_(mark4_reader_.size_input_word()),
    did_work(true),
    n_bytes_per_input_word(n_bytes_per_input_word_) {
  channel_extractor_.connect_to(mark4_reader_.get_output_buffer());

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
  compid << inputid.str() << "_mark4reader";
  monid << compid.str() << "_monitor_state";
  mark4reader_state_.init(monid.str());
  mark4reader_state_.add_property(inputid.str(), "is_a", "inputnode");
  mark4reader_state_.add_property(inputid.str(), "has", compid.str() );
  mark4reader_state_.add_property(compid.str(), "is_a", "mark4_reader");
  mark4reader_state_.add_property(compid.str(), "has", monid.str() );
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


void
Input_node_tasklet_implementation::
do_task() {
  did_work = false;

  RT_STAT( dotask_state_.begin_measure() );
  mark4_reader_timer_.resume();
  if (mark4_reader_.has_work()) {

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

bool
Input_node_tasklet_implementation::
has_work() {
  return did_work;
}

void
Input_node_tasklet_implementation::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    integer_delay_[i]->set_delay_table(table);
  }

  did_work = true;
}

void
Input_node_tasklet_implementation::
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

int
Input_node_tasklet_implementation::
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
int
Input_node_tasklet_implementation::
get_current_time() {
  return mark4_reader_.get_current_time();
}
void
Input_node_tasklet_implementation::
set_stop_time(int time) {
  did_work = true;

  for (size_t i=0; i<integer_delay_.size(); i++) {
    assert(integer_delay_[i] != NULL);
    integer_delay_[i]->set_stop_time(int64_t(1000)*time);
  }
  return mark4_reader_.set_stop_time(int64_t(1000)*time);
}

void
Input_node_tasklet_implementation::
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
