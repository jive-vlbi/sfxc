#define VERBOSE
#include "utils_bench.h"
#include "channel_extractor_interface.h"



class Benchmark {
  enum ORDER {
    CHANNEL_ORDER,
    FAN_OUT_ORDER,
    RANDOM_ORDER,
    EOL_ORDER
  };
public:
  Benchmark(Channel_extractor_interface &channel_extractor_);

  // Compare the output of the channel extractor with the brute force extractor
  bool test();

  // Benchmark the channel extractor
  double benchmark(int input_elements, int repeat);
  void do_benchmark(int n_channels, int fan_out,
                    ORDER ordering, int n_input_samples_to_process, int repeat);
  Channel_extractor_interface& get_extractor();
private:


  bool do_test(int n_channels, int fan_out,
               ORDER ordering,
               int n_input_samples_to_process);
  void initialise(int n_channels, int fan_out, ORDER ordering,
                  std::vector< std::vector<int> >& track_positions);

  bool check_output_buffers(unsigned char * in1,
                            unsigned char * in2,
                            unsigned char * out1[],
                            unsigned char * out2[],
                            int n_channels,
                            int output_bytes_per_channel,
                            int size_of_input_word,
                            int bytes_in_data1,
                            int offset_in_input_samples,
                            int offset,
                            int input_size,

                            std::vector< std::vector<int> >& track_positions);

  Channel_extractor_interface &channel_extractor;
};
