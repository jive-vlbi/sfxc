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

  int n_input_samples = input_element.buffer->data.size();
  if (n_input_samples != samples_per_block *N) {
    DEBUG_MSG(n_input_samples <<" != " << samples_per_block << " * " <<N);
    std::cout << RANK_OF_NODE << " : " << n_input_samples <<" != " << samples_per_block << " * " <<N<<"\n";
  }
  SFXC_ASSERT(n_input_samples == samples_per_block*N);

  // dechannelized output buffer
  Output_buffer_element  output_element;
  output_element.channel_data = input_element.buffer;
  output_element.start_time = input_element.start_time;
  output_element.invalid_samples_begin =
        input_element.invalid_bytes_begin*8/bits_per_sample;
  output_element.nr_invalid_samples =
        input_element.nr_invalid_bytes*8/bits_per_sample;
  SFXC_ASSERT(output_element.nr_invalid_samples >= 0);
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
