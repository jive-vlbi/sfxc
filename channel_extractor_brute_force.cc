#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>


Channel_extractor_brute_force::Channel_extractor_brute_force() {}

void
Channel_extractor_brute_force::
initialise(const std::vector< std::vector<int> > &track_positions_,
           int size_of_one_input_word_,
           int input_sample_size_) {
  track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_;

}

void
Channel_extractor_brute_force::
extract(char *in_data1,
        char *in_data2,
        int samples_in_data1, /* <= size_of_one_input_word+1 */
        char **output_data,
        int offset) {
  int sample = 0;
  for (int i=0; i<track_positions.size(); i++) {
    for (int j=0; j<input_sample_size * track_positions[0].size()/8; j++) {
      output_data[i][j] = sample; sample++;
    }
  }
}

