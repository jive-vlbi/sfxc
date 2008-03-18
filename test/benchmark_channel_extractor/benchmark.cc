#include "benchmark.h"
#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils_bench.h"
#include "timer.h"

Benchmark::Benchmark(Channel_extractor_interface &channel_extractor_)
    : channel_extractor(channel_extractor_) {}

bool Benchmark::test() {
  bool ok = true;
  bool early_exit = true;
  std::cout << ".";
  std::cout.flush();
  ok &= do_test(4,4,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(8,4,FAN_OUT_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(8,2,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(8,8,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(16,4,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(16,4,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(16,4,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  std::cout << ".";
  std::cout.flush();
  ok &= do_test(16,4,CHANNEL_ORDER, 512);
  if (early_exit && !ok) exit(0);

  return ok;
}


double Benchmark::benchmark(int datasize, int repeat) {
  do_benchmark(4,4, CHANNEL_ORDER,datasize, repeat);
  do_benchmark(8,4, CHANNEL_ORDER,datasize, repeat);
  do_benchmark(16,4, CHANNEL_ORDER,datasize, repeat);
  return 1;
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
  initialise(n_channels, fan_out, ordering, track_positions);
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
                                    &output_brute[0]);

    for (int offset_in_input_samples=0; offset_in_input_samples<10; offset_in_input_samples++) {
      // recompute the output
      randomize_buffers(output, n_output_bytes_per_channel);
      channel_extractor.extract(&in_data1[size_input_word*offset_in_input_samples],
                                &in_data2[0],
                                n_input_samples_to_process+1-offset_in_input_samples,
                                &output[0]);

      // std::cout << "offset: " << offset << std::endl;
      // print_output_buffers(output, n_output_bytes_per_channel);

      // check the result:
      result &= check_output_buffers(&output[0],
                                     &output_brute[0],
                                     n_channels,
                                     n_output_bytes_per_channel,
                                     track_positions);
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

void Benchmark::initialise(int n_channels,
                           int fan_out,
                           ORDER ordering,
                           std::vector<std::vector<int> >& track_positions) {
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
}

void Benchmark::do_benchmark(int n_channels, int fan_out,
                             ORDER ordering,
                             int n_input_samples_to_process,
                             int repeat) {

}

bool Benchmark::check_output_buffers(unsigned char * out1[],
                                     unsigned char * out2[],
                                     int n_channels,
                                     int output_bytes_per_channel,
                                     std::vector<std::vector<int> >& track_positions) {
  bool failure=false;
  for (int i=0; i<n_channels; i++) {
    for (int j=0; j<output_bytes_per_channel; j++) {
      if (out1[i][j] != out2[i][j]) {
        failure=true;
      }
    }
  }

  if (failure) {
    std::cout << std::endl;
    std::cout << "Failure detected :"<< std::endl;
    std::cout << "     subbands: " << track_positions.size() << std::endl;
    std::cout << "      fan out: " << track_positions[0].size() << std::endl;
    std::cout << " output_byte_per_channel: " << output_bytes_per_channel << std::endl;

    std::cout << "track_positions: " << std::endl;
    for (unsigned int i=0;i<track_positions.size();i++) {
      std::cout << "Channel "<<i<<": ";
      for (unsigned int j=0;j<track_positions[i].size();j++) {
        std::cout << track_positions[i][j] << ", ";
      }
      std::cout << std::endl;
    }
  }
  return !failure;
}


Channel_extractor_interface& Benchmark::get_extractor() {
  return channel_extractor;
}
