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

#include "mark4_header.h"

// At least be able to buffer the data 
// for MAX_SUBBANDS for two seconds at 256Mbps
template <class Type>
Channel_extractor<Type>::Channel_extractor()
  : output_memory_pool_(32000*MAX_SUBBANDS),
    N(sizeof(Type)) {
  // bit shift right with padding
  for (int shift=0; shift<8; shift++) {
    for (int value=0; value<256; value++) {
      bit_shift_right_table[shift][value] =
        (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
    }
  }
}

template <class Type>
Channel_extractor<Type>::~Channel_extractor() {}

template <class Type>
void
Channel_extractor<Type>::do_task() {
  assert(has_work());
  assert(output_buffers_.size() == tracks_in_subbands.size());

  // Number of output streams
  size_t n_subbands = tracks_in_subbands.size();

  assert(n_subbands > 0);
  // Number of data-tracks per output stream
  size_t n_tracks_per_subband = tracks_in_subbands[0].size();

  // The struct containing the data for processing
  Input_buffer_element input_element = input_buffer_->front();

  // Check if we only need to release the buffer
  if (input_element.only_release_data1) {
    assert(!input_element.data1.released());
    input_element.data1.release();
    input_buffer_->pop();
    return;
  }

  assert((size_t)input_element.sample_offset < input_element.data1->size());

  // The number of input samples to process
  int n_input_samples = input_element.number_data_samples;
  assert(n_input_samples > 0);

  // Number of bytes in the output chunk
  assert((n_input_samples*n_tracks_per_subband)%8==0);
  int n_output_bytes = (n_input_samples*n_tracks_per_subband)/8;
  assert(n_output_bytes > 0);

  std::vector<Output_buffer_element>  output_elements;
  { // Acquire output buffers
    output_elements.resize(n_subbands);
    for (size_t i=0; i<n_subbands; i++) {
      assert(!output_memory_pool_.empty());
      output_elements[i] = output_memory_pool_.allocate();
    }
  }

  // Do the actual channel extraction
  for (size_t subband=0; subband < n_subbands; subband++) {

    // allocate the right amount of memory for each output block
    if (output_elements[subband]->data.size() != (size_t)n_output_bytes) {
      output_elements[subband]->data.resize(n_output_bytes);
    }
    assert(output_elements[subband]->data.size() == (size_t)n_output_bytes);
    // set output_positions to the beginning of the memory block
    char *output_position = &(output_elements[subband]->data[0]);

    memset((void *)output_position, 0, n_input_samples*fan_out/8);


    char last_sample = 0;

    { // Channel extract
      if (input_element.sample_offset+input_element.number_data_samples+1 <=
          (int)input_element.data1.data().size()) {
        // everything fits in one data block
        // We don't use data2, check that it is not filled
        assert(input_element.data2.released());

        int current_bit=0;
        channel_extract(input_element.number_data_samples,
                        current_bit,
                        &input_element.data1.data()[input_element.sample_offset],
                        output_position,
                        &lookup_table[subband][0][0]);
        assert(current_bit == 0);
        current_bit=0;
        char *last_sample_ptr = &last_sample;
        channel_extract(1,
                        current_bit,
                        &input_element.data1.data()[input_element.sample_offset+
                                                    input_element.number_data_samples],
                        last_sample_ptr,
                        &lookup_table[subband][0][0]);
      } else {
        assert(!input_element.data2.released());
        int samples_in_data1 = (int)input_element.data1.data().size() -
                               input_element.sample_offset;
        int samples_in_data2 = input_element.number_data_samples -
                               samples_in_data1;

        int current_bit=0;
        channel_extract(samples_in_data1,
                        current_bit,
                        &input_element.data1.data()[input_element.sample_offset],
                        output_position,
                        &lookup_table[subband][0][0]);
        channel_extract(samples_in_data2,
                        current_bit,
                        &input_element.data2.data()[0],
                        output_position,
                        &lookup_table[subband][0][0]);
        assert(current_bit == 0);
        current_bit=0;
        char *last_sample_ptr = &last_sample;
        channel_extract(1,
                        current_bit,
                        &input_element.data2.data()[samples_in_data2],
                        last_sample_ptr,
                        &lookup_table[subband][0][0]);
      }
    }

    // Do the offset
    int subsample_offset = input_element.subsample_offset;
    {
      uint8_t *mask_right = bit_shift_right_table[subsample_offset];

      char *data = &output_elements[subband]->data[0];
      int n_output_samples = n_input_samples*fan_out/8;
      // Start at 1 because we handle the last sample differently
      for (int sample=1; sample<n_output_samples; sample++) {
        *data = (*data << subsample_offset) | mask_right[(uint8_t)*(data+1)];
        data++;
      }

      // Process the last sample
      // shift the last sample
      if (fan_out<8)
        last_sample = (last_sample<<(8-2*fan_out));

      *data = (*data << subsample_offset) | mask_right[(uint8_t)last_sample];
    }
  }

  { // release the buffers
    input_buffer_->pop();
    for (size_t i=0; i<n_subbands; i++) {
      assert(output_buffers_[i] != Output_buffer_ptr());
      output_buffers_[i]->push(output_elements[i]);
    }
  }
}

template <class Type>
bool
Channel_extractor<Type>::has_work() {
  if (tracks_in_subbands.size() == 0)
    return false;
  if (input_buffer_ == Input_buffer_ptr())
    return false;
  if (input_buffer_->empty())
    return false;
  if (output_memory_pool_.number_free_element() < output_buffers_.size())
    return false;

  return true;
}

template <class Type>
void
Channel_extractor<Type>::
set_parameters(const Input_node_parameters &input_node_param,
               const std::vector< std::vector<int> > &track_positions) {
  tracks_in_subbands = track_positions;

  // Check that all subbands have the same data rate
  assert(tracks_in_subbands.size() > 0);
  for (size_t i=0; i > tracks_in_subbands.size(); i++) {
    assert(tracks_in_subbands[0].size() == tracks_in_subbands[i].size());
  }

  if (output_buffers_.size() != tracks_in_subbands.size()) {
    output_buffers_.resize(tracks_in_subbands.size());
    for (size_t i=0; i<output_buffers_.size(); i++) {
      if (output_buffers_[i] == Output_buffer_ptr()) {
        output_buffers_[i] = Output_buffer_ptr(new Output_buffer());
      }
    }
  }


  n_subbands = track_positions.size();
  fan_out = track_positions[0].size();
  samples_per_byte = 8/fan_out;
  assert(fan_out <= 8);
  assert(8%fan_out == 0);
  assert(fan_out*samples_per_byte == 8);

  assert(n_subbands <= MAX_SUBBANDS);
  memset(lookup_table, 0, N*256*MAX_SUBBANDS);
  // Lookup table for the tracks
  for (int subband=0; subband < n_subbands; subband++) {
    for (int track_nr=0; track_nr < fan_out; track_nr++) {
      int track = track_positions[subband][track_nr];
      int n = track/8;
      assert(n < N);
      int bit = track % 8;
      for (int sample=0; sample<256; sample++) {
        if (((sample >> bit)& 1) != 0) {
          lookup_table[subband][n][sample] |= (1<<(fan_out-1-track_nr));
        }
      }
    }
  }
}


template <class Type>
void
Channel_extractor<Type>::connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}

template <class Type>
typename Channel_extractor<Type>::Output_buffer_ptr
Channel_extractor<Type>::get_output_buffer(size_t stream) {
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


template <class Type>
void
Channel_extractor<Type>::
channel_extract(int n_input_samples,
                int &curr_bit,
                const Type *in_data,
                char *&output_data,
                const uint8_t *lookup_table) {
  const uint8_t *in_pos = (const uint8_t *)in_data;

  for (int sample=0; sample<n_input_samples; sample++) {
    // samples_per_byte-1 times:
    process_sample(in_pos, output_data, lookup_table);
    curr_bit += fan_out;
    if (curr_bit == 8) {
      curr_bit = 0;
      output_data++;
    } else {
      *output_data = (*output_data << fan_out);
    }
  }
}

#endif // CHANNEL_EXTRACTOR_IMPL_H
