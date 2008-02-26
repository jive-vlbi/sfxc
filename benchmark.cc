#include "benchmark.h"
#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils.h"
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

double Benchmark::benchmark() {}

bool Benchmark::do_test(int n_channels, int fan_out,
                        ORDER ordering,
                        int n_input_samples_to_process) {
  bool result = true;
  assert((fan_out*n_channels) % 8 == 0);
  // Size of one input data sample in bytes
  const int size_input_sample = fan_out*n_channels/8;
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
  const int input_size = 2*size_input_sample*n_input_samples_to_process;
  char in_data[input_size];
  for (int i=0; i<input_size; i++) {
    in_data[i] = random();
  }

  // Allocate the output buffers
  std::vector<char *> output, output_brute;
  const int n_output_bytes_per_channel = fan_out*n_input_samples_to_process/8;
  output.resize(n_channels);
  output_brute.resize(n_channels);
  for (int i=0; i<n_channels; i++) {
    output[i] = new char[n_output_bytes_per_channel];
    output_brute[i] = new char[n_output_bytes_per_channel];
  }

  // Initialise the extractors
  channel_extractor.initialise(track_positions,
                               size_input_sample,
                               n_input_samples_to_process);
  channel_extractor_brute.initialise(track_positions,
                                     size_input_sample,
                                     n_input_samples_to_process);

  for (int offset=0; offset<fan_out; offset++) {
    // Brute force for the reference
    randomize_buffers(output_brute, n_output_bytes_per_channel);
    channel_extractor_brute.extract(&in_data[0], &in_data[0],
                                    n_input_samples_to_process+1,
                                    &output_brute[0],
                                    offset);

    for (int samples_in_data1=n_input_samples_to_process+1;
         samples_in_data1>n_input_samples_to_process-10; samples_in_data1--) {
      // recompute the output
      randomize_buffers(output, n_output_bytes_per_channel);
      channel_extractor.extract(&in_data[0],
                                &in_data[samples_in_data1*size_input_sample],
                                samples_in_data1,
                                &output[0],
                                offset);
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
    }
  }

  // Clear up the buffers and return
  for (int i=0; i<n_channels; i++) {
    delete[] output[i];
    delete[] output_brute[i];
  }
  return result;
}

bool Benchmark::check_output_buffers(char * out1[],
                                     char * out2[],
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

//
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
//
//    char in_data1[OUTPUT_SAMPLE_SIZE], in_data2[OUTPUT_SAMPLE_SIZE];
//    char *output_data1[N_SUBBANDS];
//    char *output_data2[N_SUBBANDS];
//
//
//    // Initialising the buffers:
//    for (int i=0; i<OUTPUT_SAMPLE_SIZE; i++) in_data1[i] = rand();
//
//    for (int i=0; i<N_SUBBANDS; i++) {
//      // two bits samples
//      output_data1[i] = new char[OUTPUT_SAMPLE_SIZE/4];
//      output_data2[i] = new char[OUTPUT_SAMPLE_SIZE/4];
//    }
//
//    { // Normal execution:
//      const int size_of_one_input_word = 4;
//      const int offset = 0;
//      const int n_tracks = 4;
//      const int fan_out = 4;
//      std::vector< std::vector<int> > track_positions;
//      track_positions.resize(N_SUBBANDS);
//      int track=0;
//      for (size_t i=0; i<track_positions.size(); i++) {
//        track_positions[i].resize(n_tracks);
//        for (size_t j=0; j<track_positions[i].size(); j++) {
//          track_positions[i][j] = track;
//          track++;
//        }
//      }
//      assert(track <= 8*size_of_one_input_word);
//
//      int input_sample_size = OUTPUT_SAMPLE_SIZE/track_positions[0].size();
//      check(track_positions,
//            size_of_one_input_word, input_sample_size,
//            offset,
//            in_data1, in_data2, OUTPUT_SAMPLE_SIZE,
//            output_data1, output_data2);
//
//    }
//  }
//
//private:
//  void check(std::vector< std::vector<int> > &track_positions,
//             int size_input_word,
//             int input_sample_size,
//             int offset,
//             char *in_data1, char *in_data2, int samples_in_data1,
//             char **output_data1, char **output_data2) {
//    const int n_subbands = track_positions[0].size();
//    const int fan_out = track_positions[0].size();
//
//    Extractor1 ch_ex1(track_positions, size_input_word, input_sample_size);
//    Extractor2 ch_ex2(track_positions, size_input_word, input_sample_size);
//
//    ch_ex1.extract(in_data1, in_data2, samples_in_data1,
//                   output_data1, offset);
//    ch_ex2.extract(in_data1, in_data2, samples_in_data1,
//                   output_data2, offset);
//
//    for (int subband=0; subband < n_subbands
//    for (int i=0; i<input_sample_size*fan_out/8; i++) {
//      assert(output_data1[i] == output_data2[i]);
//    }
//
//  }
//
