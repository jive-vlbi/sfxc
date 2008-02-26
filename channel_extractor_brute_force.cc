#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>


Channel_extractor_brute_force::Channel_extractor_brute_force() {}

void Channel_extractor_brute_force::initialise(const std::vector< std::vector<int> > &track_positions,
    int size_of_one_input_word,
    int input_sample_size) {}

void Channel_extractor_brute_force::extract(char *in_data1,
    char *in_data2,
    int samples_in_data1, /* <= size_of_one_input_word+1 */
    char **output_data,
    int offset) {
}

