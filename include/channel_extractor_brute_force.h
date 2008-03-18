#include "channel_extractor_interface.h"

class Channel_extractor_brute_force : public Channel_extractor_interface {
public:
  Channel_extractor_brute_force();

  void initialise(const std::vector< std::vector<int> > &track_positions_,
                  int size_of_one_input_word_,
                  int input_sample_size_);

  void extract(unsigned char *in_data1,
               unsigned char *in_data2,
               int samples_in_data1, /* <= size_of_one_input_word+1 */
               unsigned char **output_data);
private:
  
  void extract_element(unsigned char *in_data,
                       unsigned char **output_data, 
                       int output_sample);
  
  std::vector< std::vector<int> > track_positions;
  int size_of_one_input_word;
  int input_sample_size;
  
  // Computed
  int fan_out;
  
  // temporary buffer for the last sample
  unsigned char **output_data_tmp;
};
