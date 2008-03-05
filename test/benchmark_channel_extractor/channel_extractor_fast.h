#ifndef CHANNEL_EXTRACTOR_FAST_H__
#define CHANNEL_EXTRACTOR_FAST_H__

#include "channel_extractor_interface.h"

class Channel_extractor_fast : public Channel_extractor_interface {
public:
  Channel_extractor_fast();

  void initialise(const std::vector< std::vector<int> > &track_positions_,
                  int size_of_one_input_word_,
                  int input_sample_size_);

  void extract(unsigned char *in_data1,
               unsigned char *in_data2,
               int samples_in_data1, /* <= size_of_one_input_word+1 */
               unsigned char **output_data,
               int offset);
private:
  Channel_extractor_interface* hidden_implementation_;

  std::vector< std::vector<int> > track_positions;
  int size_of_one_input_word;
  int input_sample_size;
  int n_subbands;

  // Computed
  int fan_out;
};

#endif // CHANNEL_EXTRACTOR_5_H__
