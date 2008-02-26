#include "benchmark.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils.h"
#include "timer.h"


const int OUTPUT_SAMPLE_SIZE = 1024;
const int N_SUBBANDS = 8;


Benchmark::Benchmark(Channel_extractor_interface &channel_extractor_)
    : channel_extractor(channel_extractor_) {
}

bool Benchmark::test() {
}

double Benchmark::benchmark() {
  
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
