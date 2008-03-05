#include "utils.h"
#include <vector>

bool check_buffers(std::vector<char *> &output_buffers,
                   std::vector<char *> &output_buffers2,
                   int buffer_size) {
  assert(output_buffers.size() == output_buffers2.size());
  assert(buffer_size > 0);
  for (size_t i=0; i<output_buffers.size(); i++) {
    for (int j=0; j<buffer_size; j++) {
      if (output_buffers[i][j] != output_buffers2[i][j]) {
        std::cout << "mismatch at subband " << i << " byte " << j << std::endl;
        return false;
      }
    }
  }
  return true;
}

void print(std::vector<char *> &output_buffers,
           int buffer_size) {
#ifdef VERBOSE
  assert(buffer_size > 0);
  //  for (size_t j=0; j<output_buffers.size(); j++)
  //    if (j<10)
  //      std::cout << "0" << j << " ";
  //    else
  //      std::cout << j << " ";
  //  std::cout << std::endl;
  //  for (size_t j=0; j<output_buffers.size(); j++)
  //    std::cout << "---";
  //  std::cout << std::endl;
  for (int j=0; j<buffer_size; j++) {
    for (size_t i=0; i<output_buffers.size(); i++) {
      print_hex(std::cout, output_buffers[i][j]);
      std::cout << " ";
    }
    std::cout << std::endl;
  }
#endif // VERBOSE

}

void randomize_buffers(std::vector<unsigned char *> &output_buffers,
                       int buffer_size) {
  for (size_t i=0; i<output_buffers.size(); i++) {
    for (int j=0; j<buffer_size; j++) {
      output_buffers[i][j] = random();
      output_buffers[i][j] = 0xff;
    }
  }
}

void print_output_buffers(std::vector<unsigned char *> &output_buffers,
                          int buffer_size) {
  std::cout << std::hex;
  for (size_t i=0; i<output_buffers.size(); i++) {
    for (int j=0; j<buffer_size; j++) {
      if (output_buffers[i][j] < 16) std::cout << 0;
      std::cout << (int)output_buffers[i][j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::dec;
}
