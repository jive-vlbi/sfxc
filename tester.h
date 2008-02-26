#define VERBOSE

#include <iostream>
#include <vector>
#include <assert.h>

#include "channel_extractor_interface.h"
#include "channel_extractor1.h"

#include "utils.h"
#include "timer.h"

const int OUTPUT_SAMPLE_SIZE = 1024;
const int N_SUBBANDS = 8;

template <class Extractor1, class Extractor2>
class Test {
public:
  Test() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;

    char in_data1[OUTPUT_SAMPLE_SIZE], in_data2[OUTPUT_SAMPLE_SIZE];
    char *output_data1[N_SUBBANDS];
    char *output_data2[N_SUBBANDS];

    
    // Initialising the buffers:
    for (int i=0; i<OUTPUT_SAMPLE_SIZE; i++) in_data1[i] = rand();

    for (int i=0; i<N_SUBBANDS; i++) {
      // two bits samples
      output_data1[i] = new char[OUTPUT_SAMPLE_SIZE/4];
      output_data2[i] = new char[OUTPUT_SAMPLE_SIZE/4];
    }

    { // Normal execution:
      const int size_of_one_input_word = 4;
      const int offset = 0;
      const int n_tracks = 4;
      const int fan_out = 4;
      std::vector< std::vector<int> > track_positions;
      track_positions.resize(N_SUBBANDS);
      int track=0;
      for (size_t i=0; i<track_positions.size(); i++) {
        track_positions[i].resize(n_tracks);
        for (size_t j=0; j<track_positions[i].size(); j++) {
          track_positions[i][j] = track;
          track++;
        }
      }
      assert(track <= 8*size_of_one_input_word);

      int input_sample_size = OUTPUT_SAMPLE_SIZE/track_positions[0].size();
      check(track_positions,
            size_of_one_input_word, input_sample_size, 
            offset,
            in_data1, in_data2, OUTPUT_SAMPLE_SIZE,
            output_data1, output_data2);

    }
  }
  
private:
  void check(std::vector< std::vector<int> > &track_positions,
             int size_input_word,
             int input_sample_size,
             int offset,
             char *in_data1, char *in_data2, int samples_in_data1,
             char **output_data1, char **output_data2) {
    const int n_subbands = track_positions[0].size();
    const int fan_out = track_positions[0].size();

    Extractor1 ch_ex1(track_positions, size_input_word, input_sample_size);
    Extractor2 ch_ex2(track_positions, size_input_word, input_sample_size);
    
    ch_ex1.extract(in_data1, in_data2, samples_in_data1,
                   output_data1, offset);
    ch_ex2.extract(in_data1, in_data2, samples_in_data1,
                   output_data2, offset);

    for (int subband=0; subband < n_subbands
    for (int i=0; i<input_sample_size*fan_out/8; i++) {
      assert(output_data1[i] == output_data2[i]);
    }

  }

};

// void generate_output(Channel_extractor_interface &channel_extractor,
//                      Test_output &output) {
//   output.data1.resize(256);
// }

// void test() {
//   Test_output reference_output;
//   {
//     Channel_extractor1 ch_extractor;
//     generate_output(ch_extractor, reference_output);
//   }

//   {
//     Test_output test_output;
//     Channel_extractor1 ch_extractor;
//     generate_output(ch_extractor, test_output);
//     assert(test_output == reference_output);
//   }

// //   int n_subbands = 8;
// //   int fan_out    = 2;
// //   int in_size    = 1024;
// //   int size_of_one_input_word = 4;

// //   assert(n_subbands * fan_out <= in_size);
// //   assert(offset < fan_out);

// //   // Input buffer
// //   char in_data[size_of_one_input_word*(in_size+1)];
// //   for (int i=0; i<=in_size; i++)
// //     in_data[i] = (i<in_size/2 ? i : -i);
// //   for (int i=0; i<=in_size; i++)
// //     in_data[i] = i;
// //   for (int i=0; i<=in_size; i++)
// //     in_data[i] = (1<<(i%(8*size_of_one_input_word)));
// //   for (int i=0; i<=in_size; i++)
// //     in_data[i] = (0xaaaaaaaa) + i;
// //   for (int i=0; i<=in_size; i++)
// //     in_data[i] = rand();

// //   char *output_data[n_subbands];
// //   for (int i=0; i<n_subbands; i++)
// //     output_data[i] = new char[in_size*fan_out/8];

// //   if (true) { // thorough check
// //     for (int n_subbands=1; n_subbands<8; n_subbands++) {

// //       int fan_out=1;
// //       while ((fan_out <= 8) && (n_subbands * fan_out < size_of_one_input_word*8)) {
// //         // Tracks
// //         std::vector< std::vector<int> > tracks_in_subbands;
// //         tracks_in_subbands.resize(n_subbands);
// //         for (int i=0; i<n_subbands; i++)
// //           tracks_in_subbands[i].resize(fan_out);
// //         assert(n_subbands*fan_out <= size_of_one_input_word*8);
// //         for (int i=0; i<n_subbands*fan_out; i++)
// //           tracks_in_subbands[i%n_subbands][i/n_subbands] = i;
// //         for (int i=0; i<n_subbands*fan_out; i++)
// //           tracks_in_subbands[i/fan_out][i%fan_out] = i;
// //         for (int i=0; i<n_subbands*fan_out; i++)
// //           tracks_in_subbands[i/fan_out][i%fan_out] = rand()%(8*size_of_one_input_word);

// //         Channel_extractor1 extractor1(tracks_in_subbands,
// //                                       size_of_one_input_word,
// //                                       in_size);

// //         for (int offset = 0; offset < fan_out; offset++) {
// // #ifdef VERBOSE
// //           std::cout << "(subband, fan_out, offset) = "
// //           << "(" << n_subbands << ", " << fan_out << ", " << offset << ")" << std::endl;
// // #endif // VERBOSE

// //           extractor1.extract(in_data, in_data, 1025, output_data, offset);
// //         }
// //         fan_out *= 2;
// //       }
// //     }
// //   }
// }
