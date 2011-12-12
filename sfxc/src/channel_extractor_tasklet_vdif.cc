/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema <keimpema@jive.nl>, 2010
 *
 * $Id: channel_extractor_vdif.h 440 2007-12-16 20:26:04Z kruithof $
 *
 */
#include "channel_extractor_tasklet_vdif.h"

// Increase the size of the output_memory_pool_ to allow more buffering
// (8M/SIZE_MK5A_FRAME=) 400 input blocks is 1 second of data
Channel_extractor_tasklet_VDIF::
Channel_extractor_tasklet_VDIF(int samples_per_block, int N_):Channel_extractor_tasklet(samples_per_block,N_)
{
  SFXC_ASSERT(N_ > 0);
  init_stats();
  last_duration_=0;
}

Channel_extractor_tasklet_VDIF::~Channel_extractor_tasklet_VDIF()
{
}

void
Channel_extractor_tasklet_VDIF::do_task() {
  // Number of output streams, one output stream corresponds to one subband
  SFXC_ASSERT(n_subbands == output_buffers_.size());
  SFXC_ASSERT(n_subbands > 0);

  // The struct containing the data for processing
  Input_buffer_element &input_element = input_buffer_->front();

#if 0
  int n_input_samples = input_element.buffer->data.size();
  if (n_input_samples != samples_per_block *N) {
    DEBUG_MSG(n_input_samples <<" != " << samples_per_block << " * " <<N);
    std::cout << RANK_OF_NODE << " : " << n_input_samples <<" != " << samples_per_block << " * " <<N<<"\n";
  }
  SFXC_ASSERT(n_input_samples == samples_per_block*N);
#endif

  // dechannelized output buffer
  Output_buffer_element  output_element;
  output_element.channel_data = input_element.buffer;
  output_element.start_time = input_element.start_time;
  int n_invalid_blocks = input_element.invalid.size();
  output_element.invalid.resize(n_invalid_blocks);
  for(int i = 0 ; i < n_invalid_blocks ; i++){
    output_element.invalid[i].invalid_begin = input_element.invalid[i].invalid_begin;
    output_element.invalid[i].nr_invalid = input_element.invalid[i].nr_invalid;
    SFXC_ASSERT(output_element.invalid[i].nr_invalid >= 0);
  }
  data_processed_ +=  input_element.buffer->data.size();

  int chan = input_element.channel;
  if(chan >=0){
    SFXC_ASSERT(output_buffers_[chan] != Output_buffer_ptr());
    output_buffers_[chan]->push(output_element);
  }else{
    // Broadcast to all channels (used for invalid data)
    for(int i=0;i<output_buffers_.size();i++)
      output_buffers_[i]->push(output_element);
  }
  input_buffer_->pop();
}

void
Channel_extractor_tasklet_VDIF::
set_parameters(const Input_node_parameters &input_node_param){
  n_subbands = input_node_param.channels.size();
  bits_per_sample = input_node_param.bits_per_sample();
  fan_out    = bits_per_sample *
               input_node_param.subsamples_per_sample();
  std::cout << "n_subbands = " << n_subbands << ", bits_per_sample = " << bits_per_sample << ", fan_out = " << fan_out <<"\n";
}

