#define VERBOSE

#include "channel_extractor_interface.h"

class Benchmark {
public:
  Benchmark(Channel_extractor_interface &channel_extractor_);

  // Compare the output of the channel extractor with the brute force extractor
  bool test();

  // Benchmark the channel extractor
  double benchmark();

private:
  enum ORDER {
    CHANNEL_ORDER,
    FAN_OUT_ORDER,
    RANDOM_ORDER
  };

  bool do_test(int n_channels, int fan_out,
               ORDER ordering,
               int n_input_samples_to_process);

  bool check_output_buffers(unsigned char * out1[],
                            unsigned char * out2[],
                            int n_channels,
                            int bytes_per_channel);

  Channel_extractor_interface &channel_extractor;
};
