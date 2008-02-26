#ifndef CHANNEL_EXTRACTOR1_H
#define CHANNEL_EXTRACTOR1_H

#include "channel_extractor_interface.h"

class Channel_extractor1 : public Channel_extractor_interface {
public:
  Channel_extractor1(const std::vector< std::vector<int> > &track_positions,
                     int size_of_one_input_word,
                     int input_sample_size)
    : Channel_extractor_interface(track_positions, 
                                  size_of_one_input_word,
                                  input_sample_size) {
  };
  void extract(char *in_data1,
               char *in_data2,
               int samples_in_data1, /* <= size_of_one_input_word+1 */
               char **output_data,
               int offset) {};
};

#endif // CHANNEL_EXTRACTOR1_H
