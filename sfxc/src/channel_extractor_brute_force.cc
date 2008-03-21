#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils.h"


Channel_extractor_brute_force::Channel_extractor_brute_force() {
name_ = "Channel_extractor_brute_force";
}

void
Channel_extractor_brute_force::
initialise(const std::vector< std::vector<int> > &track_positions_,
           int size_of_one_input_word_,
           int input_sample_size_) {
  track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_;
  assert((input_sample_size_%2) == 0);

  fan_out = track_positions[0].size();

  output_data_tmp = (unsigned char **)malloc(sizeof(unsigned char*) *
                    track_positions.size());
  for (size_t i=0; i<track_positions.size(); i++) {
    output_data_tmp[i] = new unsigned char();
  }
}

void
Channel_extractor_brute_force::
extract(unsigned char *in_data1,
        unsigned char *in_data2,
        int samples_in_data1, /* <= size_of_one_input_word+1 */
        unsigned char **output_data) {
  int output_sample = 0, bit = 0;
  for (int input_sample=0; input_sample < input_sample_size; input_sample++) {
    if (bit == 0) { // Clear a new sample
      for (size_t track=0; track<track_positions.size(); track++) {
        output_data[track][output_sample] = 0;
      }
    }
    // Extract the data
    if (input_sample < samples_in_data1) {
      extract_element(&in_data1[size_of_one_input_word*input_sample],
                      output_data, output_sample);
    } else {
      extract_element(&in_data2[size_of_one_input_word *
                                (input_sample-samples_in_data1)],
                      output_data, output_sample);
    }
    // Move the output pointer
    bit += fan_out;
    if (bit == 8) {
      output_sample ++;
      bit = 0;
    }
    assert(bit < 8);
  }
}

void
Channel_extractor_brute_force::
extract_element(unsigned char *in_data,
                unsigned char **output_data, int output_sample) {
  for (size_t track=0; track<track_positions.size(); track++) {
    for (int sample=0; sample<fan_out; sample++) {
      int pos = track_positions[track][sample];
      // bit shift
      output_data[track][output_sample] *= 2;
      // extract data
      output_data[track][output_sample] |= (in_data[pos/8]>>(pos%8))&1;
    }
  }
}