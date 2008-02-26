#ifndef CHANNEL_EXTRACTOR_INTERFACE_H
#define CHANNEL_EXTRACTOR_INTERFACE_H

#include <vector>

class Channel_extractor_interface {
public:
  // Size of one input word to process in the extract function
  // Track position array
  // size of one input sample in bytes
  Channel_extractor_interface(const std::vector< std::vector<int> > &track_positions,
                              int size_of_one_input_word,
                              int input_sample_size) {};

  // extract samples 0..(samples_in_data1-1) from data1
  // extract samples samples_in_data1..size_of_one_input_word from data2
  // in_data1 is not aligned with the sequence number

  // #define MAX_NR_SUBBANDS 32
  // nr_subbands (for the output) = track_positions.size()
  // fan_out = track_positions[0].size() == track_positions[i].size()
  // fan_out is the number of output bits per input sample
  // fan_out is 1 2 4 8
  // offset is 0 .. fan_out-1

  virtual void extract(char *in_data1,
                       char *in_data2,
                       int samples_in_data1, /* <= size_of_one_input_word+1 */
                       char **output_data,
                       int offset)=0;

};

#endif // CHANNEL_EXTRACTOR_INTERFACE_H
