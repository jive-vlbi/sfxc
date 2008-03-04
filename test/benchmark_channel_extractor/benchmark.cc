#include "benchmark.h"
#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils_bench.h"
#include "timer.h"


const int OUTPUT_SAMPLE_SIZE = 1024;
const int N_SUBBANDS = 8;


Benchmark::Benchmark(Channel_extractor_interface &channel_extractor_)
    : channel_extractor(channel_extractor_) {}

bool Benchmark::test() {
  bool ok = true;
  ok &= do_test(4,4,CHANNEL_ORDER,1024);
  ok &= do_test(8,4,CHANNEL_ORDER,1024);
  ok &= do_test(8,2,CHANNEL_ORDER,1024);
  ok &= do_test(8,8,CHANNEL_ORDER,1024);
  ok &= do_test(16,4,CHANNEL_ORDER,1024);
  ok &= do_test(16,4,CHANNEL_ORDER,512);
  ok &= do_test(16,4,CHANNEL_ORDER,64);
  ok &= do_test(16,4,CHANNEL_ORDER,32);
  return ok;
}

double Benchmark::benchmark() {
  return -1;
}

bool Benchmark::do_test(int n_channels, int fan_out,
                        ORDER ordering,
                        int n_input_samples_to_process) {
  bool result = true;
  assert((fan_out*n_channels) % 8 == 0);
  // Size of one input data sample in bytes
  const int size_input_word = fan_out*n_channels/8;
  Channel_extractor_brute_force channel_extractor_brute;

  // Initialise the track positions
  std::vector< std::vector<int> > track_positions;
  track_positions.resize(n_channels);
  for (int i=0; i<n_channels; i++)
    track_positions[i].resize(fan_out);
  switch (ordering) {
    case CHANNEL_ORDER: {
      int track=0;
      for (int i=0; i<n_channels; i++) {
        for (int j=0; j<fan_out; j++) {
          track_positions[i][j] = track;
          track++;
        }
      }
      break;
    }
    case FAN_OUT_ORDER: {
      int track=0;
      for (int j=0; j<fan_out; j++) {
        for (int i=0; i<n_channels; i++) {
          track_positions[i][j] = track;
          track++;
        }
      }
      break;
    }
    case RANDOM_ORDER: {
      assert(false);
    }
  }

  // Initialise the input data
  const int input_size = size_input_word*(n_input_samples_to_process+1);
  unsigned char in_data1[input_size],in_data2[input_size];

  // Allocate the output buffers
  std::vector<unsigned char *> output, output_brute;
  const int n_output_bytes_per_channel = fan_out*n_input_samples_to_process/8;
  output.resize(n_channels);
  output_brute.resize(n_channels);
  for (int i=0; i<n_channels; i++) {
    output[i] = new unsigned char[n_output_bytes_per_channel];
    output_brute[i] = new unsigned char[n_output_bytes_per_channel];
  }

  // Initialise the extractors
  channel_extractor.initialise(track_positions,
                               size_input_word,
                               n_input_samples_to_process);
  channel_extractor_brute.initialise(track_positions,
                                     size_input_word,
                                     n_input_samples_to_process);

  assert(input_size == size_input_word*(n_input_samples_to_process+1));

  for (int offset=0; offset<fan_out; offset++) {
    // randomize input data:
    for (int i=0; i<input_size; i++) {
      in_data1[i] = random();
      in_data2[i] = random();
    }

    // Brute force for the reference
    randomize_buffers(output_brute, n_output_bytes_per_channel);
    channel_extractor_brute.extract(&in_data1[0], &in_data2[0],
                                    n_input_samples_to_process+1,
                                    &output_brute[0],
                                    offset);

    for (int offset_in_input_samples=0;
         offset_in_input_samples<10; offset_in_input_samples++) {
      // recompute the output
      randomize_buffers(output, n_output_bytes_per_channel);
      channel_extractor.extract(&in_data1[size_input_word*offset_in_input_samples],
                                &in_data2[0],
                                n_input_samples_to_process+1-offset_in_input_samples,
                                &output[0],
                                offset);

      // std::cout << "offset: " << offset << std::endl;
      // print_output_buffers(output, n_output_bytes_per_channel);
      
      // check the result:
      result &= check_output_buffers(&output[0],
                                     &output_brute[0],
                                     n_channels,
                                     n_output_bytes_per_channel);
      if (!result) {
        for (int i=0; i<n_channels; i++) {
          delete[] output[i];
          delete[] output_brute[i];
        }
        return result;
      }

      { // shift all samples one input position:
        for (int i=input_size-1; i>=size_input_word; i--)
          in_data2[i] = in_data2[i-size_input_word];
        for (int i=size_input_word-1; i>=0; i--)
          in_data2[i] = in_data1[input_size-size_input_word+i];
        for (int i=input_size-1; i>=size_input_word; i--)
          in_data1[i] = in_data1[i-size_input_word];
      }
    }
  }

  // Clear up the buffers and return
  for (int i=0; i<n_channels; i++) {
    delete[] output[i];
    delete[] output_brute[i];
  }
  return result;
}

bool Benchmark::check_output_buffers(unsigned char * out1[],
                                     unsigned char * out2[],
                                     int n_channels,
                                     int bytes_per_channel) {
  for (int i=0; i<n_channels; i++) {
    for (int j=0; j<bytes_per_channel; j++) {
      if (out1[i][j] != out2[i][j]) {
        std::cout << "Samples differ, subband = " << i
        << " sample = " << j << std::endl;
        std::cout << std::hex << (unsigned int)out1[i][j]<< " != " << (unsigned int)out2[i][j]
        << std::dec << std::endl;
        return false;
      }
    }
  }
  return true;
}
