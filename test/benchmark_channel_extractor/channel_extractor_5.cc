/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - SFXC/SCARIe project.
 * This file contains:
 *   - Implementation of a channel extractor with look up table.
 */
#include <cassert>
#include <iostream>
#include "channel_extractor_5.h"

template<int size_of_one_input_word_>
class Channel_extractor_5_impl : public Channel_extractor_interface {
public:
  Channel_extractor_5_impl() {}

  void initialise(const std::vector< std::vector<int> > &track_positions,
                  int IGNORED_size_of_one_input_word,
                  int input_sample_size) {


    // bit shift right with padding
    for (int shift=0; shift<8; shift++) {
      for (int value=0; value<256; value++) {
        bit_shift_right_table[shift][value] =
          (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
      }
    }

    input_sample_size_ = input_sample_size;
    n_subbands = track_positions.size();
    fan_out = track_positions[0].size();
    samples_per_byte = 8/fan_out;
    assert(fan_out <= 8);
    assert(8%fan_out == 0);
    assert(fan_out*samples_per_byte == 8);

    memset(lookup_table, 0, size_of_one_input_word_*256*8);
    // Lookup table for the tracks
    for (int subband=0; subband < n_subbands; subband++) {
      for (int track_nr=0; track_nr < fan_out; track_nr++) {
        int track = track_positions[subband][track_nr];
        int n = track/8;
        assert(n < size_of_one_input_word_);
        int bit = track % 8;
        for (int sample=0; sample<256; sample++) {
          if (((sample >> bit)& 1) != 0) {
            lookup_table[subband][n][sample] |= (1<<(fan_out-1-track_nr));
          }
        }
      }
    }

    output_data_tmp = (unsigned char **)malloc(sizeof(unsigned char*) * n_subbands );
    for (size_t i=0; i<track_positions.size(); i++) {
      output_data_tmp[i] = new unsigned char();
    }
  }

  void extract(unsigned char *in_data1,
               unsigned char *in_data2,
               int samples_in_data1, /* <= size_of_one_input_word+1 */
               unsigned char **output_data) {
    int offset = 0;
    int difference = (input_sample_size_+1) - samples_in_data1;
    int out_position=0;
    //std::cout << "\n V1  AT: " << samples_in_data1 << " diff: " << samples_per_byte<< std::endl;
    do_task(samples_in_data1, offset, in_data1, output_data, out_position);
    //std::cout << "\n V2 AT: " << out_position << " in pos: " << size_of_one_input_word_*difference << std::endl;
    //do_task(difference, offset, &in_data2[0], output_data, out_position );
  };


  void do_task(int n_input_samples,
               int offset,
               unsigned char in_data[],
               unsigned char** output_data, int& out_position) {

    for (int subband=0; subband<n_subbands; subband++) {
      const uint8_t *in_pos = (const uint8_t *)in_data;
      unsigned char *out_pos = &(output_data[subband][out_position]);
      const uint8_t *table = &lookup_table[subband][0][0];
      memset((void *)out_pos, 0, n_input_samples*fan_out/8+1);

      for (int sample=0; sample<n_input_samples; sample+=samples_per_byte) {
        // samples_per_byte-1 times:
        for (int i=1; i<samples_per_byte; i++) {
          process_sample(in_pos, out_pos, table);
          *out_pos = (*out_pos << fan_out);
        }

        process_sample(in_pos, out_pos, table);
        out_pos++;
      }
      if ( offset != 0 ) { // Do the offset
        uint8_t *mask_right = bit_shift_right_table[offset];

        // shift the last sample
        if (fan_out < 4)
          *out_pos = (*out_pos<<(8-2*fan_out));

        unsigned char *data = output_data[subband];
        for (int sample=0; sample<n_input_samples*fan_out/8; sample++) {
          *data = (*data << offset) | mask_right[(uint8_t)*(data+1)];
          data++;
        }
      }
    }
    out_position += n_input_samples/samples_per_byte;
  }

private:
  inline void process_sample(const uint8_t *&in_array,
                             unsigned char *output_data,
                             const uint8_t *table) {
    for (int n=0; n<size_of_one_input_word_; n++) {
      *output_data |= table[*in_array];
      table += 256;
      in_array++;
    }
  }
  // Lookup table for the channel extraction
  // At most 8 channels
  // lookup_table[index in Type word][value of the byte][output sample per channel]
  uint8_t lookup_table[8][size_of_one_input_word_][256];

  // Lookup table for a right bitshift with 0 inserted
  uint8_t bit_shift_right_table[8][256];

  // Fan out
  int fan_out, samples_per_byte;

  int n_subbands;
  int input_sample_size_;

  unsigned char** output_data_tmp;
};

Channel_extractor_5::Channel_extractor_5() {
  name_ = "Channel_extractor_5";
  hidden_implementation_ = NULL;
}

void Channel_extractor_5::initialise(const std::vector< std::vector<int> > &track_positions_,
                                     int size_of_one_input_word_,
                                     int input_sample_size_) {
  track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_+1;
  fan_out = track_positions[0].size();
  n_subbands = track_positions.size();

  switch (size_of_one_input_word) {
  case 1:
    hidden_implementation_ = new Channel_extractor_5_impl<1>();
    break;
  case 2:
    hidden_implementation_ = new Channel_extractor_5_impl<2>();
    break;
  case 4:
    hidden_implementation_ = new Channel_extractor_5_impl<4>();
    break;
  case 8:
    hidden_implementation_ = new Channel_extractor_5_impl<8>();
    break;
  default:
    std::cerr << " size_of_input_word: " << size_of_one_input_word << std::endl;
    assert(false&&"Unsupported size of input word for channelization");
  }
  hidden_implementation_->initialise(track_positions, size_of_one_input_word_, input_sample_size);
}

void Channel_extractor_5::extract(unsigned char *in_data1,
                                  unsigned char *in_data2,
                                  int samples_in_data1, /* <= size_of_one_input_word+1 */
                                  unsigned char **output_data,
                                  int offset) {
  assert( hidden_implementation_ != NULL && "big" );

  hidden_implementation_->extract(in_data1, in_data2, samples_in_data1-1, output_data);
}

