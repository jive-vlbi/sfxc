#define VERBOSE

#include <iostream>

#include "channel_extractor_interface.h"
#include "channel_extractor_brute_force.h"

#include "benchmark.h"

int main() {
  Channel_extractor_brute_force channel_extractor_bf;
  
  Benchmark benchmark(channel_extractor_bf);

}
