/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 440 2007-12-16 20:26:04Z kruithof $
 *
 */

#include "channel_extractor_tasklet.h"
#include "channel_extractor_brute_force.h"
#include "channel_extractor_5.h"
#include "channel_extractor_fast.h"
#include "channel_extractor_dynamic.h"


#include "mark5a_header.h"

//#define USE_EXTRACTOR_5

// Increase the size of the output_memory_pool_ to allow more buffering
// (8M/SIZE_MK5A_FRAME=) 400 input blocks is 1 second of data
Channel_extractor_tasklet::
Channel_extractor_tasklet(int samples_per_block, int N_)
    : output_memory_pool_(400*MAX_SUBBANDS),
    n_subbands(0),
    fan_out(0),
    N(N_), samples_per_block(samples_per_block) {
  SFXC_ASSERT(N_ > 0);

  init_stats();

  last_duration_=0;


#ifdef USE_EXTRACTOR_5
  ch_extractor = new Channel_extractor_5();
#else
  /// This one is much more better as it will select
  /// dynamically the best channelizer that can handle the
  /// input data-stream.
  ch_extractor = new Channel_extractor_dynamic();
#endif //USE_EXTRACTOR_5

#ifdef RUNTIME_STATISTIC
  std::stringstream inputid;
  std::stringstream chexid;
  std::stringstream monid;

  inputid << "inputnode" << RANK_OF_NODE;
  chexid << inputid.str() << "_channelextractor";
  monid << chexid.str() << "_monitor_speed";

  monitor_.init(monid.str(), 1000, "stats/");
  monitor_.add_property(inputid.str(), "is_a", "inputnode");
  monitor_.add_property(inputid.str(), "has", chexid.str() );
  monitor_.add_property(chexid.str(), "is_a", "channel_extractor");
  monitor_.add_property(chexid.str(), "has", monid.str() );

#endif //RUNTIME_STATISTIC

}

void Channel_extractor_tasklet::init_stats() {
  data_processed_ = 0;
  timer_waiting_input_.restart();
  timer_waiting_output_.restart();
  timer_processing_.restart();
}

Channel_extractor_tasklet::~Channel_extractor_tasklet() {
  /// Print statistics info
  double wait_duration = (timer_waiting_input_.measured_time()+timer_waiting_output_.measured_time());
  double total_duration = wait_duration+timer_processing_.measured_time();

  if ( total_duration >= 1.0 ) {
    double ratio1 = ((100.0*timer_processing_.measured_time())/total_duration);
    double ratio2 = ((100.0*timer_waiting_input_.measured_time())/total_duration);
    double ratio3 = ((100.0*timer_waiting_output_.measured_time())/total_duration);
    PROGRESS_MSG( "channelizing speed:" <<  1.0*toMB(data_processed_)/total_duration
                  << "MB/s" << " processing:"<< ratio1 <<"% input:"<< ratio2 <<"% output:"<< ratio3 <<"%");
    //init_stats();
  }
}

void Channel_extractor_tasklet::do_execute() {
  /// The thread is in a running state
  isrunning_ = true;

  /// Exception handler to insure to catch the QueueClosedException event
  /// This Exception is thrown when the current thread is blocked on a
  /// queue.front() operation and that the queue is closed on its other side.
  try {

    /// Main thread loop
    while ( isrunning_ && !input_buffer_->isclose() ) {
      do_task();
    }

    /// The queue has been closed and is empty.
  } catch (QueueClosedException&e ) {
    DEBUG_MSG("My input queue has been closed ! I should probably stop then");
  }

  /// If the input queue is not empty something was wrong.
  /// To much data has been sent to dechannelization.
  ///SFXC_ASSERT( input_buffer_->empty() );

  /// We close our own output queues
  for (unsigned int i=0;i<output_buffers_.size();i++) {
    output_buffers_[i]->close();
  }
}

void Channel_extractor_tasklet::stop() {
  isrunning_=false;
}

void
Channel_extractor_tasklet::do_task() {
#ifdef RUNTIME_STATISTIC
  monitor_.begin_measure();
#endif // RUNTIME_STATISTIC

  // Number of output streams, one output stream corresponds to one subband
  SFXC_ASSERT(n_subbands == output_buffers_.size());
  SFXC_ASSERT(n_subbands > 0);

  // The struct containing the data for processing
  // This is the not-yet-channelized data.
  timer_waiting_input_.resume();
  Input_buffer_element &input_element = input_buffer_->front();
  timer_waiting_input_.stop();


  // The number of input samples to process
  // For mark5a this is 1 block
  // For mark5b this is N_MK5B_BLOCKS_TO_READ as the input_element also contains
  //   a time in microseconds and not all mark5b blocks start on an integer number
  //   of microseconds
  int n_input_samples = input_element.data().buffer.size();
  if (n_input_samples != samples_per_block *N) {
    DEBUG_MSG(n_input_samples <<" != " << samples_per_block << " * " <<N);
  }
  SFXC_ASSERT(n_input_samples == samples_per_block*N);

  // Number of bytes in the output chunk
  SFXC_ASSERT((n_input_samples*fan_out)%8==0);
  int n_output_bytes = (samples_per_block*fan_out)/8;
  SFXC_ASSERT(n_output_bytes > 0);

  // Array of dechannelized output buffers
  Output_buffer_element  output_elements[n_subbands];
  // Array of pointers to the actual output data arrays
  unsigned char *output_positions[n_subbands];
  { // Acquire output buffers
    timer_waiting_output_.resume();
    for (size_t subband=0; subband<n_subbands; subband++) {
      output_elements[subband].channel_data = output_memory_pool_.allocate();

      output_elements[subband].start_time = input_element.data().start_time;

      // allocate the right amount of memory for each output block
      if (output_elements[subband].channel_data.data().data.size() !=
          (size_t)n_output_bytes) {
        output_elements[subband].channel_data.data().data.resize(n_output_bytes);
      }
      SFXC_ASSERT(output_elements[subband].channel_data.data().data.size() ==
                  (size_t)n_output_bytes);

      output_positions[subband] =
        (unsigned char *)&(output_elements[subband].channel_data.data().data[0]);

      // Copy the invalid-data members
      // Mark5a-files have headers, which should be invalidated
      SFXC_ASSERT(input_element->invalid_bytes_begin >= 0);
      output_elements[subband].invalid_samples_begin =
        input_element->invalid_bytes_begin*fan_out/bits_per_sample;
      output_elements[subband].nr_invalid_samples =
        input_element->nr_invalid_bytes*fan_out/(bits_per_sample*N);
      SFXC_ASSERT(output_elements[subband].nr_invalid_samples >= 0);
    }
    timer_waiting_output_.stop();
  }

  // Channel extract
  // This is done in a separate class to allow for different optimizations
  timer_processing_.resume();
  ch_extractor->extract((unsigned char *) &input_element.data().buffer[0],
                        output_positions);
  timer_processing_.stop();

  data_processed_ +=  input_element.data().buffer.size();

  { // release the input buffer and put the output buffer
    for (size_t i=0; i<n_subbands; i++) {
      SFXC_ASSERT(output_buffers_[i] != Output_buffer_ptr());
      output_buffers_[i]->push(output_elements[i]);
    }
    input_buffer_->pop();
  }

  /*
   /// Print statistics info
   double wait_duration = (timer_waiting_input_.measured_time()+timer_waiting_output_.measured_time());
   double total_duration = wait_duration+timer_processing_.measured_time();

   if( total_duration >= last_duration_ + 2.0 )
   {
     double ratio1 = ((100.0*timer_processing_.measured_time())/total_duration);
     double ratio2 = ((100.0*timer_waiting_input_.measured_time())/total_duration);
     double ratio3 = ((100.0*timer_waiting_output_.measured_time())/total_duration);
     PROGRESS_MSG( "channelizing speed:" <<  1.0*toMB(data_processed_)/total_duration
            << "MB/s" << " processing:"<< ratio1 <<"%, input:"<< ratio2 <<"%, output:"<< ratio3 <<"%");
     //init_stats();
     last_duration_ = total_duration;
   }
  */

#ifdef RUNTIME_STATISTIC
  monitor_.end_measure(n_output_bytes*n_subbands);
#endif // RUNTIME_STATISTIC
}


bool
Channel_extractor_tasklet::has_work() {
  if (n_subbands == 0) {
    //    DEBUG_MSG_RANK(3, "subbands not initialised");
    return false;
  }
  if (input_buffer_ == Input_buffer_ptr()) {
    //    DEBUG_MSG_RANK(3, "input_buffer_ not initialised");
    return false;
  }
  if (input_buffer_->empty()) {
    //    DEBUG_MSG_RANK(3, "input_buffer_ empty");
    return false;
  }
  if (output_memory_pool_.number_free_element() < output_buffers_.size()) {
    //    DEBUG_MSG_RANK(3, "output memory pool full "
    //                   << output_memory_pool_.number_free_element()
    //                   << " < "
    //                   << output_buffers_.size());
    return false;
  }

  return true;
}


void
Channel_extractor_tasklet::
set_parameters(const Input_node_parameters &input_node_param,
               const std::vector< std::vector<int> > &track_positions) {
  n_subbands = input_node_param.channels.size();
  bits_per_sample = input_node_param.bits_per_sample();
  fan_out    = bits_per_sample *
               input_node_param.subsamples_per_sample();

  ch_extractor->initialise(track_positions, N, samples_per_block);
  DEBUG_MSG("Using channel extractor: " << ch_extractor->name() );
}



void
Channel_extractor_tasklet::connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}


Channel_extractor_tasklet::Output_buffer_ptr
Channel_extractor_tasklet::get_output_buffer(size_t stream) {
  if (stream >= output_buffers_.size()) {
    output_buffers_.resize(stream+1);
    SFXC_ASSERT(stream < output_buffers_.size());
  }
  if (output_buffers_[stream] == Output_buffer_ptr()) {
    output_buffers_[stream] = Output_buffer_ptr(new Output_buffer());
  }
  SFXC_ASSERT(output_buffers_[stream] != Output_buffer_ptr());
  return output_buffers_[stream];
}

void Channel_extractor_tasklet::empty_input_queue() {
  while (!input_buffer_->empty()) {
    input_buffer_->pop();
  }
}

