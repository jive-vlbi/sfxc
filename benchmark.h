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
  Channel_extractor_interface &channel_extractor;
};

