/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 440 2007-12-16 20:26:04Z kruithof $
 *
 */

#ifndef CHANNEL_EXTRACTOR_IMPL_H
#define CHANNEL_EXTRACTOR_IMPL_H

#include "channel_extractor_tasklet.h"
#include "channel_extractor_brute_force.h"
#include "mark4_header.h"

// At least be able to buffer the data
// for MAX_SUBBANDS for two seconds at 256Mbps
template <class Type>
Channel_extractor_tasklet<Type>::Channel_extractor_tasklet()
    : output_memory_pool_(32000*MAX_SUBBANDS),
    ch_extractor(new Channel_extractor_brute_force()),
    n_subbands(0),
fan_out(0) {}

template <class Type>
Channel_extractor_tasklet<Type>::~Channel_extractor_tasklet() {}

template <class Type>
void
Channel_extractor_tasklet<Type>::do_task() {
  assert(has_work());

  // Number of output streams
  assert(n_subbands == output_buffers_.size());
  assert(n_subbands > 0);

  // The struct containing the data for processing
  Input_buffer_element input_element = input_buffer_->front();

  // The number of input samples to process
  int n_input_samples = input_element.data().mk4_data.size();
  assert(n_input_samples == SIZE_MK4_FRAME);

  // Number of bytes in the output chunk
  assert((n_input_samples*fan_out)%8==0);
  int n_output_bytes = (n_input_samples*fan_out)/8;
  assert(n_output_bytes > 0);

  Output_buffer_element  output_elements[n_subbands];
  unsigned char *output_positions[n_subbands];
  { // Acquire output buffers
    for (size_t subband=0; subband<n_subbands; subband++) {
      assert(!output_memory_pool_.empty());
      output_elements[subband].channel_data = output_memory_pool_.allocate();

      output_elements[subband].start_time = input_element.data().start_time;

      // allocate the right amount of memory for each output block
      if (output_elements[subband].channel_data.data().data.size() !=
          (size_t)n_output_bytes) {
        output_elements[subband].channel_data.data().data.resize(n_output_bytes);
      }
      assert(output_elements[subband].channel_data.data().data.size() == (size_t)n_output_bytes);

      output_positions[subband] = (unsigned char *)&(output_elements[subband].channel_data.data().data[0]);
    }
  }

  { // Channel extract
    ch_extractor->extract((unsigned char *) &input_element.data().mk4_data[0],
                          (unsigned char *) NULL,
                          n_input_samples,
                          output_positions);
  }


  { // release the buffers
    input_element.release();
    input_buffer_->pop();
    for (size_t i=0; i<n_subbands; i++) {
      assert(output_buffers_[i] != Output_buffer_ptr());
      output_buffers_[i]->push(output_elements[i]);
    }
  }
}

template <class Type>
bool
Channel_extractor_tasklet<Type>::has_work() {
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

template <class Type>
void
Channel_extractor_tasklet<Type>::
set_parameters(const Input_node_parameters &input_node_param,
               const std::vector< std::vector<int> > &track_positions) {
  n_subbands = input_node_param.channels.size();
  fan_out    = input_node_param.bits_per_sample() *
               input_node_param.subsamples_per_sample();
  ch_extractor->initialise(track_positions, sizeof(Type), 20000);
}


template <class Type>
void
Channel_extractor_tasklet<Type>::connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}

template <class Type>
typename Channel_extractor_tasklet<Type>::Output_buffer_ptr
Channel_extractor_tasklet<Type>::get_output_buffer(size_t stream) {
  if (stream >= output_buffers_.size()) {
    output_buffers_.resize(stream+1);
    assert(stream < output_buffers_.size());
  }
  if (output_buffers_[stream] == Output_buffer_ptr()) {
    output_buffers_[stream] = Output_buffer_ptr(new Output_buffer());
  }
  assert(output_buffers_[stream] != Output_buffer_ptr());
  return output_buffers_[stream];
}

#endif // CHANNEL_EXTRACTOR_IMPL_H
