#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <iostream>
#include <assert.h>

template <class Type>
std::ostream &print_hex(std::ostream &out, const Type &t) {
#ifdef VERBOSE
#if 0
  out << std::hex;
  for (int i=2*sizeof(Type)-1; i>=0; i--) {
    out << ((t>>(4*i))&15);
  }
  out << std::dec;
#else

  for (int i=8*sizeof(Type)-1; i>=0; i--) {
    out << ((t>>i)&1);
    if ((i>0) && (i%8==0))
      out << ".";
  }
#endif
#endif // VERBOSE
  return out;
}

bool check_buffers(std::vector<char *> &output_buffers,
                   std::vector<char *> &output_buffers2,
                   int buffer_size);


void print(std::vector<char *> &output_buffers,
           int buffer_size);

void randomize_buffers(std::vector<unsigned char *> &output_buffers,
                       int buffer_size);
void print_output_buffers(std::vector<unsigned char *> &output_buffers,
                          int buffer_size);
#endif // UTILS_H
