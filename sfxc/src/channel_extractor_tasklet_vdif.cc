/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema <keimpema@jive.nl>, 2010
 *
 */
#include "channel_extractor_tasklet_vdif.h"

// Increase the size of the output_memory_pool_ to allow more buffering
// (8M/SIZE_MK5A_FRAME=) 400 input blocks is 1 second of data
Channel_extractor_tasklet_VDIF::
Channel_extractor_tasklet_VDIF(Data_format_reader_ptr reader)
  : Channel_extractor_tasklet(reader)
{
  init_stats();
  last_duration_ = 0;
  num_channel_extractor_threads = 0;
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

  // dechannelized output buffer
  Output_buffer_element  output_element;
  output_element.channel_data = input_element.buffer;
  output_element.start_time = input_element.start_time;
  size_t num_invalid_blocks = input_element.invalid.size();
  output_element.invalid.resize(input_element.invalid.size());
  for (size_t i = 0 ; i < input_element.invalid.size(); i++) {
    output_element.invalid[i].invalid_begin = input_element.invalid[i].invalid_begin;
    output_element.invalid[i].nr_invalid = input_element.invalid[i].nr_invalid;
    SFXC_ASSERT(output_element.invalid[i].nr_invalid >= 0);
  }
  data_processed_ += input_element.buffer->data.size();

  SFXC_ASSERT(output_buffers_[input_element.channel] != Output_buffer_ptr());
  output_buffers_[input_element.channel]->push(output_element);
  input_buffer_->pop();
}

void
Channel_extractor_tasklet_VDIF::
set_parameters(const Input_node_parameters &param) {
  n_subbands = param.channels.size();
}
