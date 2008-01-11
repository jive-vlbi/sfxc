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

template <class Type>
Channel_extractor<Type>::Channel_extractor() {}

template <class Type>
Channel_extractor<Type>::~Channel_extractor() {}

template <class Type>
void
Channel_extractor<Type>::
process_samples(Type *input_buffer,
                int n_input_samples,
                int start_track_nr,
                int end_track_nr,
                std::vector<char *> &output_buffer,
                int &output_buffer_bit,
                int &output_buffer_byte) {
  // for each input sample
  for (int input_sample = 0; input_sample < n_input_samples; input_sample++) {

    // for each track in the output subband
    for (int track_nr = start_track_nr; track_nr < end_track_nr; track_nr++) {
      // for each subband
      for (size_t subband=0; subband < output_buffer.size(); subband++) {
        // bit shift
        output_buffer[subband][output_buffer_byte] =
          (output_buffer[subband][output_buffer_byte]<<1);
        // add new sample
        output_buffer[subband][output_buffer_byte] +=
          (((*input_buffer) >> tracks[subband][track_nr]) & 1);
      }

      // go to the next bit
      output_buffer_bit ++;
      if (output_buffer_bit==8) { // Compute modulo 8
        output_buffer_bit = 0;
        output_buffer_byte ++;
      }
    }
    input_buffer++;
  }
}

template <class Type>
void
Channel_extractor<Type>::do_task() {
  assert(has_work());
  assert(output_buffers_.size() == tracks.size());

  // Number of output streams
  size_t n_tracks = tracks.size();

  // Number of data-tracks per output stream
  size_t n_tracks_per_channel = tracks[0].size();

  // The struct containing the data for processing
  Input_buffer_element                input_element;
  // The actual data buffer
  Type *                              input_position;
  // The number of input samples to process
  int                                 n_input_samples;
  { // Acquire input buffer
    input_element = input_buffer_->front();
    assert((size_t)input_element.sample_offset < input_element.data1->size());
    input_position = &input_element.data1.data()[input_element.sample_offset];
    n_input_samples = input_element.number_data_samples;
    assert(n_input_samples > 0);
  }

  // Number of bytes in the output chunk
  assert((n_input_samples*n_tracks_per_channel)%8==0);
  int n_output_bytes = (n_input_samples*n_tracks_per_channel)/8;
  assert(n_output_bytes > 0);

  std::vector<Output_buffer_element>  output_elements;
  std::vector<char *>                 output_positions;
  { // Acquire output buffers
    output_elements.resize(n_tracks);
    output_positions.resize(n_tracks);
    for (size_t i=0; i<n_tracks; i++) {
      assert(!output_memory_pool_.empty());
      output_elements[i] = output_memory_pool_.allocate();
      // allocate the right amount of memory for each output block
      if (output_elements[i]->data.size() != (size_t)n_output_bytes) {
        output_elements[i]->data.resize(n_output_bytes);
      }
      assert(output_elements[i]->data.size() > 0);
      // set output_positions to the beginning of the memory block
      output_positions[i] = &(output_elements[i]->data[0]);
    }
  }

  // The current bit we want to fill
  int bit_position = 0;      // The bit number in the output sample
  int byte_position = 0;      // The bit number in the output sample

  // Process the first sample
  process_samples(input_position,
                  /* number of input samples */ 1,
                  input_element.subsample_offset, n_tracks_per_channel,
                  output_positions,
                  bit_position, byte_position);
  input_position++;

  if ((size_t)(input_element.number_data_samples+input_element.sample_offset) <
      input_element.data1.data().size()) {
    // everything fits in one data block
    // We don't use data2, check that it is not filled
    assert(input_element.data2.released());

    // process the second sample to the second last sample
    process_samples(input_position,
                    /* number of input samples */ input_element.number_data_samples-1,
                    0, n_tracks_per_channel,
                    output_positions,
                    bit_position, byte_position);
    input_position += input_element.number_data_samples-2;

    assert(! input_element.release_data1);
  } else {
    assert(!input_element.data2.released());

    // process the second sample to the second last sample in the first data block
    int samples_in_data1 =
      input_element.data1.data().size() - (input_element.sample_offset+1);
    if (samples_in_data1 == input_element.number_data_samples-1) {
      // Data fits in data1 except for the last sample that is processed separately
      process_samples(input_position,
                      /* number of input samples */ samples_in_data1,
                      0, n_tracks_per_channel,
                      output_positions,
                      bit_position, byte_position);
      
      input_position = &input_element.data2.data()[0];
      input_position = 0;
    } else {
      assert(samples_in_data1 < input_element.number_data_samples-1);
      
      // Read in data form data1
      process_samples(input_position,
                      /* number of input samples */ samples_in_data1,
                      0, n_tracks_per_channel,
                      output_positions,
                      bit_position, byte_position);

      // process samples in data2
      int samples_in_data2 = input_element.number_data_samples-1-samples_in_data1;
      input_position = &input_element.data2.data()[0];
      process_samples(input_position,
                      /* number of input samples */ samples_in_data2,
                      0, n_tracks_per_channel,
                      output_positions,
                      bit_position, byte_position);
      input_position += samples_in_data2;
    }
  }
  // process the last sample
  process_samples(&input_element.last_sample,
                  /* number of input samples */ 1,
                  0, input_element.subsample_offset,
                  output_positions,
                  bit_position, byte_position);

  // Easiest check
  assert(bit_position == 0);
  assert(byte_position == n_output_bytes);

  { // release the buffers
    if (input_element.release_data1)
      input_element.data1.release();

    input_buffer_->pop();
    for (size_t i=0; i<n_tracks; i++) {
      assert(output_buffers_[i] != Output_buffer_ptr());
      output_buffers_[i]->push(output_elements[i]);
    }
  }
}

template <class Type>
bool
Channel_extractor<Type>::has_work() {
  if (input_buffer_ == Input_buffer_ptr()) {
    //DEBUG_MSG("input_buffer does not exist");
    return false;
  }
  if (input_buffer_->empty()) {
    //DEBUG_MSG("input_buffer empty");
    return false;
  }
  if (output_memory_pool_.number_free_element() < output_buffers_.size()) {
    //DEBUG_MSG("Not enough output positions");
    return false;
  }

  return true;
}

template <class Type>
void
Channel_extractor<Type>::
set_parameters(const Input_node_parameters &input_node_param,
               const std::vector< std::vector<int> > &track_positions) {
  tracks = track_positions;

  // Check that all subbands have the same data rate
  assert(tracks.size() > 0);
  for (size_t i=0; i > tracks.size(); i++) {
    assert(tracks[0].size() == tracks[i].size());
  }

  if (output_buffers_.size() != tracks.size()) {
    output_buffers_.resize(tracks.size());
    for (size_t i=0; i<output_buffers_.size(); i++) {
      if (output_buffers_[i] == Output_buffer_ptr()) {
        output_buffers_[i] = Output_buffer_ptr(new Output_buffer());
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

#endif // CHANNEL_EXTRACTOR_IMPL_H
