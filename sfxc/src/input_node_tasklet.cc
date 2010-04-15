/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl>, 2008
 *
 * $Id:$
 *
 */

#include <sched.h>
#include "input_node_tasklet.h"
#include "utils.h"
#include "mark5a_reader.h"
#include "mark5b_reader.h"
#include "vlba_reader.h"
#include "vdif_reader.h"
#include "monitor.h"

typedef Input_node_types::Data_memory_pool  Data_memory_pool;
typedef boost::shared_ptr<Data_memory_pool> Data_memory_pool_ptr;
typedef boost::shared_ptr<Data_reader>      Data_reader_ptr;
Input_node_tasklet *
get_input_node_tasklet_mark5a(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                              int ref_year, int ref_day) {
  // Maximal buffer size
  Input_data_format_reader::Data_frame data;
  data.buffer = memory_pool_->allocate();

  boost::shared_ptr<Mark5a_reader> mark5a_reader_ptr =
    boost::shared_ptr<Mark5a_reader>( get_mark5a_reader(reader, data, ref_year, ref_day) );

  return new Input_node_tasklet(mark5a_reader_ptr, memory_pool_, data);
}

Input_node_tasklet *
get_input_node_tasklet_vlba(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                            int ref_year, int ref_day) {
  // Maximal buffer size
  Input_data_format_reader::Data_frame data;
  data.buffer = memory_pool_->allocate();

  boost::shared_ptr<VLBA_reader> vlba_reader_ptr =
    boost::shared_ptr<VLBA_reader>( get_vlba_reader(reader, data, ref_year, ref_day) );

  return new Input_node_tasklet(vlba_reader_ptr, memory_pool_, data);
}

Input_node_tasklet *
get_input_node_tasklet_mark5b(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                              int ref_year, int ref_day) {
  typedef boost::shared_ptr<Input_data_format_reader> Input_reader_ptr;
  Input_data_format_reader::Data_frame   data;
  data.buffer = memory_pool_->allocate();

  Input_reader_ptr   mark5b_reader_ptr(new Mark5b_reader(reader, data, ref_year, ref_day));

  return new Input_node_tasklet(mark5b_reader_ptr, memory_pool_, data);
}

Input_node_tasklet *
get_input_node_tasklet_vdif(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                            int ref_year, int ref_day) {
  typedef boost::shared_ptr<VDIF_reader> Input_reader_ptr;
  Input_data_format_reader::Data_frame   data;
  data.buffer = memory_pool_->allocate();

  Input_reader_ptr  vdif_reader_ptr(new VDIF_reader(reader, data, ref_year, ref_day));
  return new Input_node_tasklet(vdif_reader_ptr, memory_pool_, data);
}


Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type, int ref_year, int ref_day) {
  SFXC_ASSERT(type != UNINITIALISED);
  boost::shared_ptr<Data_memory_pool> memory_pool_(new Data_memory_pool(10));

  if (type == MARK5A) {
    return get_input_node_tasklet_mark5a(reader, memory_pool_,ref_year, ref_day);
  }
  if (type == VLBA) {
    return get_input_node_tasklet_vlba(reader, memory_pool_, ref_year, ref_day);
  }
  if (type == MARK5B) {
    return get_input_node_tasklet_mark5b(reader, memory_pool_, ref_year, ref_day);
  }
  if (type == VDIF) {
    return get_input_node_tasklet_vdif(reader, memory_pool_, ref_year, ref_day);
  }
  return NULL;
}



Input_node_tasklet::
Input_node_tasklet(Input_reader_ptr_ reader_ptr, Data_memory_pool_ptr memory_pool_,
                   Input_reader_::Data_frame &data)
    : reader_(reader_ptr, memory_pool_, data),
    n_bytes_per_input_word(reader_ptr->bytes_per_input_word()),
    delay_pool(10) {
  if(reader_ptr->get_transport_type()==VDIF)
    channel_extractor_= Channel_extractor_tasklet_ptr( 
        new Channel_extractor_tasklet_VDIF(reader_ptr->size_data_block() /
                                           reader_ptr->bytes_per_input_word(),
                                           reader_ptr->bytes_per_input_word()));
  else
    channel_extractor_= Channel_extractor_tasklet_ptr( 
        new Channel_extractor_tasklet(reader_ptr->size_data_block() /
                                      reader_ptr->bytes_per_input_word(),
                                      reader_ptr->bytes_per_input_word()));

  channel_extractor_->connect_to(reader_.get_output_buffer());

  last_duration_ = 0;
  initialise();
}


void
Input_node_tasklet::
add_time_interval(int32_t start_time, int32_t stop_time) {
  //SFXC_ASSERT(!integer_delay_.empty());
  //SFXC_ASSERT(integer_delay_[0] != NULL);

  SFXC_ASSERT(!delay_pool.empty());
  /// Add a list of delays to the data writers
  Delay_memory_pool_element delay_list=delay_pool.allocate();
  delay_list.data().resize(0);
  delay_list.data().push_back(get_delay((uint64_t)start_time*1000));
  get_delays((uint64_t)start_time*1000, (uint64_t)stop_time*1000-1, delay_list.data());
  data_writer_.add_delay(delay_list);
  data_writer_.add_time_interval(uint64_t(1000)*start_time, uint64_t(1000)*stop_time);

  /// A new interval is added to the mark5 reader-tasklet. It is converted into
  /// usec
  reader_.add_time_interval(uint64_t(1000)*start_time, uint64_t(1000)*stop_time);
}

void Input_node_tasklet::initialise()
{
}

Input_node_tasklet::~Input_node_tasklet() {
  channel_extractor_->empty_input_queue();
//  integer_delay_.empty_input_queue();
  data_writer_.empty_input_queue();

	PROGRESS_MSG( "Total duration:" << rttimer_processing_.measured_time() << " sec" );
	PROGRESS_MSG( "      reading:" << toMB(reader_.get_num_processed_bytes())/rttimer_processing_.measured_time() << " MB/s" );
	PROGRESS_MSG( "  channelizer:" << toMB(channel_extractor_->get_num_processed_bytes())/rttimer_processing_.measured_time() << " MB/s duration:" << channel_extractor_->get_sec() );
//	PROGRESS_MSG( "integer_delay:" << toMB(integer_delay_.get_num_processed_bytes())/rttimer_processing_.measured_time() << " MB/s" );
	PROGRESS_MSG( "      writing:" << toMB(data_writer_.get_num_processed_bytes())/data_writer_.get_sec() << " MB/s duration:" << data_writer_.get_sec() );
}


void
Input_node_tasklet::wait_termination() {
  /// Block until all the thread into the pool terminates.
  wait( pool_ );
}

void
Input_node_tasklet::start_tasklets() {
	rttimer_processing_.start();
  pool_.register_thread( data_writer_.start() );
  pool_.register_thread( channel_extractor_->start() );
  pool_.register_thread( reader_.start() );
}

void
Input_node_tasklet::stop_tasklets() {
  reader_.stop();
  channel_extractor_->stop();
  data_writer_.stop();
  rttimer_processing_.stop();
}

void Input_node_tasklet::set_delay_table(Delay_table_akima &table) {
  delay_table = table;
}

void
Input_node_tasklet::
set_parameters(const Input_node_parameters &input_node_param,
               int node_nr) {
  reader_.set_parameters(input_node_param);
  channel_extractor_->set_parameters(input_node_param,
                                     reader_.get_tracks(input_node_param));

  size_t number_frequency_channels = input_node_param.channels.size();

  sample_rate=input_node_param.sample_rate();
  bits_per_sample=input_node_param.bits_per_sample();
  int nr_output_bytes = input_node_param.number_channels*bits_per_sample/8;
  SFXC_ASSERT(((nr_output_bytes*(8/bits_per_sample))*1000000LL) % sample_rate== 0);
  delta_time = (nr_output_bytes*(8/bits_per_sample))*1000000LL/sample_rate;

	for (size_t i=0; i < number_frequency_channels; i++)
		data_writer_.add_channel();

  for (size_t i=0; i < number_frequency_channels; i++) {
    data_writer_.connect_to(i, channel_extractor_->get_output_buffer(i) );
    data_writer_.set_parameters(i, input_node_param);
  }
  // Number of samples for one integration slice
  int nr_ffts = Control_parameters::nr_ffts_per_integration_slice(input_node_param.integr_time,
                                                  sample_rate, input_node_param.number_channels);
  size_slice = nr_ffts*input_node_param.number_channels;
}


int
Input_node_tasklet::
get_current_time() {
  // Current time in [ms], if the delay correction hasn't progressed as far as
  // the reader we return the current time position of the delay correction
  int time = reader_.get_current_time()/1000;

  int writer_time = data_writer_.get_current_time()/1000;
  if (writer_time != INVALID_TIME)
    time = std::min(time, writer_time);
  return time;
}

void
Input_node_tasklet::add_data_writer(size_t i, Data_writer_sptr data_writer) {
  /// Add a new timeslice to stream to the given data_writer into the
  /// data_writer queue.
  data_writer_.add_timeslice_to_stream(i, data_writer, size_slice);
}

void
Input_node_tasklet::get_delays(uint64_t start_time, uint64_t stop_time,
                               std::vector<Delay> &delay_list)
{
  SFXC_ASSERT(stop_time>start_time);
  uint64_t dif_time = stop_time-start_time;
  Delay dstart=get_delay(start_time);
  Delay dstop=get_delay(stop_time);
  bool delay_different=((dstop.bytes!=dstart.bytes)||
                        (dstop.remaining_samples!=dstart.remaining_samples));

  if(delay_different){
    if(dif_time<3){
      delay_list.push_back(dstop);
    }else{
      get_delays(start_time,start_time+dif_time/2,delay_list);
      get_delays(start_time+dif_time/2,stop_time,delay_list);
    }
  }
}

/*void
Input_node_tasklet::get_delays(uint64_t start_time, uint64_t stop_time,
                               std::vector<Delay> &delay_list)
{
  // TODO: this can be implemented much more efficiently
  int nffts=(stop_time-start_time)/delta_time;
  uint64_t cur_time=start_time+delta_time/2;
  Delay old_delay, new_delay;
  old_delay=get_delay(cur_time);

  delay_list.resize(0);
  delay_list.push_back(old_delay);

  for(int i=1;i<nffts;i++){
    cur_time+=delta_time;
    new_delay=get_delay(cur_time);
    if((new_delay.bytes!=old_delay.bytes)||
       (new_delay.remaining_samples!=old_delay.remaining_samples)){
      delay_list.push_back(new_delay);
      old_delay=new_delay;
    }
  }
}*/

Delay
Input_node_tasklet::get_delay(int64_t time) {
  SFXC_ASSERT(delay_table.initialised());
  SFXC_ASSERT(delta_time > 0);
  SFXC_ASSERT(delta_time%2 == 0);
  double delay_ = delay_table.delay(time);
  int32_t delay_in_samples = (int32_t) std::floor(delay_*sample_rate+.5);
  SFXC_ASSERT(delay_in_samples <= 0);

  int32_t delay_in_bytes = -((-delay_in_samples)/(8/bits_per_sample))-1;
  int32_t delay_in_remaining_samples =
                    delay_in_samples-delay_in_bytes*(8/bits_per_sample);
  if (delay_in_remaining_samples*bits_per_sample == 8) {
    delay_in_bytes++;
    delay_in_remaining_samples = 0;
  }

  SFXC_ASSERT((delay_in_bytes <= 0) &&
         (delay_in_remaining_samples < 8));
  SFXC_ASSERT((delay_in_bytes*(8/bits_per_sample) +
          delay_in_remaining_samples) ==
         delay_in_samples);
  return (Delay){time, delay_in_bytes, delay_in_remaining_samples};
}
