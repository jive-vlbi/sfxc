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
                              Time ref_date) {
  boost::shared_ptr<Mark5a_reader> mark5a_reader_ptr =
    boost::shared_ptr<Mark5a_reader>( new Mark5a_reader(reader, ref_date) );

  return new Input_node_tasklet(mark5a_reader_ptr, memory_pool_);
}

Input_node_tasklet *
get_input_node_tasklet_vlba(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                            Time ref_date) {
  // Maximal buffer size
  Input_data_format_reader::Data_frame data;
  data.buffer = memory_pool_->allocate();

  boost::shared_ptr<VLBA_reader> vlba_reader_ptr =
    boost::shared_ptr<VLBA_reader>( new VLBA_reader(reader, ref_date));

  return new Input_node_tasklet(vlba_reader_ptr, memory_pool_);
}

Input_node_tasklet *
get_input_node_tasklet_mark5b(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                              Time ref_date) {
  typedef boost::shared_ptr<Input_data_format_reader> Input_reader_ptr;
  Input_data_format_reader::Data_frame   data;
  data.buffer = memory_pool_->allocate();

  Input_reader_ptr   mark5b_reader_ptr(new Mark5b_reader(reader, data, ref_date));

  return new Input_node_tasklet(mark5b_reader_ptr, memory_pool_);
}

Input_node_tasklet *
get_input_node_tasklet_vdif(Data_reader_ptr reader, Data_memory_pool_ptr memory_pool_,
                            Time ref_date) {
  typedef boost::shared_ptr<VDIF_reader> Input_reader_ptr;
  Input_data_format_reader::Data_frame   data;
  data.buffer = memory_pool_->allocate();

  Input_reader_ptr  vdif_reader_ptr(new VDIF_reader(reader, data, ref_date));
  return new Input_node_tasklet(vdif_reader_ptr, memory_pool_);
}


Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type, Time ref_date) {
  SFXC_ASSERT(type != UNINITIALISED);
  boost::shared_ptr<Data_memory_pool> memory_pool_(new Data_memory_pool(4096));

  if (type == MARK5A) {
    return get_input_node_tasklet_mark5a(reader, memory_pool_, ref_date);
  }
  if (type == VLBA) {
    return get_input_node_tasklet_vlba(reader, memory_pool_, ref_date);
  }
  if (type == MARK5B) {
    return get_input_node_tasklet_mark5b(reader, memory_pool_, ref_date);
  }
  if (type == VDIF) {
    return get_input_node_tasklet_vdif(reader, memory_pool_, ref_date);
  }
  return NULL;
}



Input_node_tasklet::
Input_node_tasklet(Input_reader_ptr_ reader_ptr, Data_memory_pool_ptr memory_pool_)
    : reader_(reader_ptr, memory_pool_), delay_pool(10) 
{
  last_duration_ = 0;
  initialized = false;
}


void
Input_node_tasklet::
add_time_interval(Time &start_time, Time &stop_time) {
  //SFXC_ASSERT(!integer_delay_.empty());
  //SFXC_ASSERT(integer_delay_[0] != NULL);

  //  SFXC_ASSERT(!delay_pool.empty());
  /// Add a list of delays to the data writers
  Delay_memory_pool_element delay_list=delay_pool.allocate();
  delay_list.data().resize(0);
  delay_list.data().push_back(get_delay(start_time));

  start_time.set_sample_rate(sample_rate);
  stop_time.set_sample_rate(sample_rate);
  int64_t nsamples = stop_time.diff_samples(start_time);
  get_delays(start_time, nsamples, delay_list.data());
  data_writer_.add_delay(delay_list);
  data_writer_.add_time_interval(start_time, stop_time);

  /// A new interval is added to the mark5 reader-tasklet. It is converted into
  /// usec
  reader_.add_time_interval(start_time, stop_time);
}

void Input_node_tasklet::initialise()
{
#if 0
  if(reader_.get_data_reader()->get_transport_type()==VDIF)
    channel_extractor_= Channel_extractor_tasklet_ptr( 
        new Channel_extractor_tasklet_VDIF(reader_.get_data_reader()->size_data_block() /
                                           reader_.size_input_word(),
                                           reader_.size_input_word()));
  else
#endif
    channel_extractor_= Channel_extractor_tasklet_ptr( 
        new Channel_extractor_tasklet(reader_.get_data_reader()->size_data_block() /
                                      reader_.size_input_word(),
                                      reader_.size_input_word()));

  channel_extractor_->connect_to(reader_.get_output_buffer());
  initialized = true;
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
  pool_.register_thread( channel_extractor_->start() );
  pool_.register_thread( reader_.start() );
}

void
Input_node_tasklet::stop_tasklets() {
  reader_.stop();
  channel_extractor_->stop();
  data_writer_.stop_threads();
  rttimer_processing_.stop();
}

void Input_node_tasklet::set_delay_table(Delay_table_akima &table) {
  delay_table = table;
}

void
Input_node_tasklet::
set_parameters(const Input_node_parameters &input_node_param,
               int station_number) {
  reader_.set_parameters(input_node_param);
  if(!initialized)
    initialise();

  channel_extractor_->set_parameters(input_node_param);

  size_t number_frequency_channels = input_node_param.channels.size();

  sample_rate=input_node_param.sample_rate();
  bits_per_sample=input_node_param.bits_per_sample();
  int nr_output_bytes = (input_node_param.fft_size * bits_per_sample) / 8;
  SFXC_ASSERT(((nr_output_bytes*(8/bits_per_sample))*1000000LL) % sample_rate== 0);
//  delta_time = (nr_output_bytes*(8/bits_per_sample))*1000000LL/sample_rate;

  for (size_t i=0; i < number_frequency_channels; i++)
    data_writer_.add_channel();

  for (size_t i=0; i < number_frequency_channels; i++) {
    data_writer_.connect_to(i, channel_extractor_->get_output_buffer(i) );
    data_writer_.set_parameters(i, input_node_param, station_number);
  }
  // Number of samples for one integration slice
  int nr_ffts = Control_parameters::
                nr_ffts_per_integration_slice((int)input_node_param.integr_time.get_time_usec(),
                                              sample_rate, input_node_param.fft_size);
  size_slice = nr_ffts * input_node_param.fft_size;
}


Time
Input_node_tasklet::
get_current_time() {
  // Current time in [ms], if the delay correction hasn't progressed as far as
  // the reader we return the current time position of the delay correction
  Time reader_time = reader_.get_current_time();

  Time writer_time = data_writer_.get_current_time();
  if (writer_time < reader_time)
    return writer_time;
  else
    return reader_time;
}

void
Input_node_tasklet::add_data_writer(size_t i, Data_writer_sptr data_writer) {
  /// Add a new timeslice to stream to the given data_writer into the
  /// data_writer queue.
  data_writer_.add_timeslice_to_stream(i, data_writer, size_slice);
}

void
Input_node_tasklet::get_delays(Time start_time, int64_t nsamples, std::vector<Delay> &delay_list)
{
  Time stop_time = start_time;
  stop_time.inc_samples(nsamples);
  SFXC_ASSERT(stop_time>start_time);
  Delay dstart = get_delay(start_time);
  Delay dstop = get_delay(stop_time);
  bool delay_different=((dstop.bytes != dstart.bytes)||
                        (dstop.remaining_samples != dstart.remaining_samples));

  if(delay_different){
    if(nsamples < 3){
      delay_list.push_back(dstop);
    }else{
      get_delays(start_time, nsamples / 2, delay_list);
      start_time.inc_samples(nsamples / 2);
      get_delays(start_time, nsamples - (nsamples / 2), delay_list);
    }
  }else{
    bool rate_different = (delay_table.rate(start_time) * delay_table.rate(stop_time)) < 0;
    if((rate_different) && (nsamples >= 3)){
      get_delays(start_time, nsamples / 2, delay_list);
      start_time.inc_samples(nsamples / 2);
      get_delays(start_time, nsamples - (nsamples / 2), delay_list);
    }
  }
}

Delay
Input_node_tasklet::get_delay(Time time) {
  SFXC_ASSERT(delay_table.initialised());
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
