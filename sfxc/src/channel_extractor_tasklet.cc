/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 440 2007-12-16 20:26:04Z kruithof $
 *
 */
#include "channel_extractor_tasklet.h"
#include "channel_extractor_5.h"
#include "channel_extractor_dynamic.h"

#include "mark5a_header.h"

//#define USE_EXTRACTOR_5

// Increase the size of the output_memory_pool_ to allow more buffering
// (8M/SIZE_MK5A_FRAME=) 400 input blocks is 1 second of data
Channel_extractor_tasklet::
Channel_extractor_tasklet(int samples_per_block, int N_)
    : output_memory_pool_(400*MAX_SUBBANDS),
    n_subbands(0),
    fan_out(0), seqno(0),
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
}

void Channel_extractor_tasklet::init_stats() {
  data_processed_ = 0;
}

Channel_extractor_tasklet::~Channel_extractor_tasklet()
{
}

// Number of threads for paralle processing in the channel extraction phase.
#ifndef NUM_CHANNEL_EXTRACTOR_THREADS
#define NUM_CHANNEL_EXTRACTOR_THREADS 1
#endif

void *
Channel_extractor_tasklet::process(void *self_)
{
  Channel_extractor_tasklet *self =
    static_cast<Channel_extractor_tasklet *>(self_);

  try {
    while (self->isrunning())
      self->do_task();
    return NULL;
  }
  catch (QueueClosedException&e ) {
    ;
  }
}

void Channel_extractor_tasklet::do_execute() {
#if NUM_CHANNEL_EXTRACTOR_THREADS > 0
  pthread_t process_thread[NUM_CHANNEL_EXTRACTOR_THREADS];
#endif

  /// The thread is in a running state
  isrunning_ = true;
  timer_.start();

#if NUM_CHANNEL_EXTRACTOR_THREADS > 0
  pthread_mutex_init(&seqno_lock, NULL);
  pthread_cond_init(&seqno_cond, NULL);
  for (int i = 0; i < NUM_CHANNEL_EXTRACTOR_THREADS; i++)
    pthread_create(&process_thread[i], NULL, process, static_cast<void*>(this));
#endif

  /// Exception handler to insure to catch the QueueClosedException event
  /// This Exception is thrown when the current thread is blocked on a
  /// queue.front() operation and that the queue is closed on its other side.
  try {

    /// Main thread loop
    while ( isrunning_ && !input_buffer_->isclose() ) {
      do_task();
    }

#if NUM_CHANNEL_EXTRACTOR_THREADS > 0
    for (int j = 0; j < NUM_CHANNEL_EXTRACTOR_THREADS; j++)
      pthread_join(process_thread[j], NULL);
#endif

    /// The queue has been closed and is empty.
  } catch (QueueClosedException&e ) {
    DEBUG_MSG("My input queue has been closed ! I should probably stop then");
  }

  // Empty input buffer
  while(!input_buffer_->empty()){
    input_buffer_->pop();
  }

  /// We close our own output queues
  for (unsigned int i=0;i<output_buffers_.size();i++) {
    output_buffers_[i]->close();
  }

  timer_.stop();
}

void Channel_extractor_tasklet::stop() {
  isrunning_=false;
}

void
Channel_extractor_tasklet::do_task() {
  // Number of output streams, one output stream corresponds to one subband
  SFXC_ASSERT(n_subbands == output_buffers_.size());
  SFXC_ASSERT(n_subbands > 0);

  // Acquire output buffers for dechannelized data first.  This may
  // block, so if we do this after grabbing an input buffer we might
  // deadlock.
  Output_buffer_element output_elements[n_subbands];
  //timer_waiting_output_.resume();
  for (size_t subband = 0; subband < n_subbands; subband++)
    output_elements[subband].channel_data = output_memory_pool_.allocate();
  //timer_waiting_output_.stop();

  // The struct containing the data for processing
  // This is the not-yet-channelized data.
  //timer_waiting_input_.resume();
  const Input_buffer_element &input_element = input_buffer_->front_and_pop();
  //timer_waiting_input_.stop();

  // The number of input samples to process
  // For mark5a this is 1 block
  // For mark5b this is N_MK5B_BLOCKS_TO_READ as the input_element also contains
  //   a time in microseconds and not all mark5b blocks start on an integer number
  //   of microseconds
  int n_input_samples = input_element.buffer->data.size();
  if (n_input_samples != samples_per_block * N) {
    DEBUG_MSG(n_input_samples <<" != " << samples_per_block << " * " <<N);
  }
  SFXC_ASSERT(n_input_samples == samples_per_block*N);

  // Number of bytes in the output chunk
  SFXC_ASSERT((n_input_samples*fan_out)%8==0);
  int n_output_bytes = (samples_per_block*fan_out)/8;
  SFXC_ASSERT(n_output_bytes > 0);

  // Array of pointers to the actual output data arrays
  unsigned char *output_positions[n_subbands];
  { // Acquire output buffers
    for (size_t subband=0; subband<n_subbands; subband++) {
      output_elements[subband].start_time = input_element.start_time;
      // allocate the right amount of memory for each output block
      if (output_elements[subband].channel_data.data().data.size() !=
          (size_t)n_output_bytes) {
        output_elements[subband].channel_data.data().data.resize(n_output_bytes);
      }
      SFXC_ASSERT(output_elements[subband].channel_data.data().data.size() ==
                  (size_t)n_output_bytes);

      output_positions[subband] =
        (unsigned char *)&(output_elements[subband].channel_data.data().data[0]);

      if ((subband2track[subband] & input_element.mask) != subband2track[subband]){
        // channel is masked out
        output_elements[subband].invalid.resize(1);
        output_elements[subband].invalid[0].invalid_begin = 0;
        int nbytes = input_element.buffer->data.size();
        output_elements[subband].invalid[0].nr_invalid = nbytes * fan_out / (8 * N);
      }else{
        // Copy the invalid-data members,
        int n_invalid_blocks = input_element.invalid.size();
        output_elements[subband].invalid.resize(n_invalid_blocks);
        for(int i = 0 ; i < n_invalid_blocks ; i++){
          SFXC_ASSERT(input_element.invalid[i].invalid_begin >= 0);
          // Simpliying assumption : invalid data starts at the beginning of the input word
          // Only for an irrelivantly small subset of input words does this not always hold
          // (for the first and last word of a 65KB invalid block if N > 4)
          output_elements[subband].invalid[i].invalid_begin =
                  input_element.invalid[i].invalid_begin * fan_out / (8*N);
          output_elements[subband].invalid[i].nr_invalid =
                  input_element.invalid[i].nr_invalid * fan_out / (8 * N);
          SFXC_ASSERT(output_elements[subband].invalid[i].nr_invalid >= 0);
        }
      }
    }
  }

  // Channel extract
  // This is done in a separate class to allow for different optimizations
  //timer_processing_.resume();
  ch_extractor->extract((unsigned char *) &input_element.buffer->data[0],
                        output_positions);

  //timer_processing_.stop();

#if NUM_CHANNEL_EXTRACTOR_THREADS > 0
  pthread_mutex_lock(&seqno_lock);
  data_processed_ += input_element.buffer->data.size();
  while (input_element.seqno != seqno)
    pthread_cond_wait(&seqno_cond, &seqno_lock);
  pthread_mutex_unlock(&seqno_lock);
#else
  data_processed_ += input_element.buffer->data.size();
#endif

  { // release the input buffer and put the output buffer
    for (size_t i=0; i<n_subbands; i++) {
      SFXC_ASSERT(output_buffers_[i] != Output_buffer_ptr());
      output_buffers_[i]->push(output_elements[i]);
    }
  }

#if NUM_CHANNEL_EXTRACTOR_THREADS > 0
  pthread_mutex_lock(&seqno_lock);
  seqno++;
  pthread_cond_broadcast(&seqno_cond);
  pthread_mutex_unlock(&seqno_lock);
#endif
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
set_parameters(const Input_node_parameters &input_node_param){
  n_subbands = input_node_param.channels.size();
  bits_per_sample = input_node_param.bits_per_sample();
  fan_out    = bits_per_sample *
               input_node_param.subsamples_per_sample();
  std::vector< std::vector<int> > track_positions;
  subband2track.resize(input_node_param.channels.size());
  memset(&subband2track[0], 0, input_node_param.channels.size() * sizeof(uint64_t));
   
  for(int i = 0; i < input_node_param.channels.size(); i++){
    track_positions.push_back(input_node_param.channels[i].tracks);
    int ntracks_channel = input_node_param.channels[i].tracks.size();
    for(int j = 0; j < ntracks_channel; j++){
      int track = input_node_param.channels[i].tracks[j];
      subband2track[i] |= (uint64_t)1 << track;
    }
  }
  ch_extractor->initialise(track_positions, N, samples_per_block, bits_per_sample);

  DEBUG_MSG("Using channel extractor: " << ch_extractor->name() );
  std::cout << RANK_OF_NODE << " : N = " << N << ", fan_out = " << fan_out << "\n";
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
