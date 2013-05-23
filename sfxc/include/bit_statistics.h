/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema, <keimpema@jive.nl>
 *
 */
#ifndef BIT_STATISTICS_H
#define BIT_STATISTICS_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include "utils.h"

class bit_statistics;
typedef boost::shared_ptr<bit_statistics>   bit_statistics_ptr;

class bit_statistics{
public:
  bit_statistics();
  ~bit_statistics();
  void reset_statistics(int bits_per_sample_, int scale_);
  void inc_counter(unsigned char word);
  void inc_invalid(int n);
  int *get_statistics();
  int bits_per_sample;
  int scale;
private:
  int nInvalid;
  std::vector<int> data_counts;
  std::vector<int> statistics;
};

inline void 
bit_statistics::inc_counter(unsigned char word){
  data_counts[(unsigned int)word]++;
}

inline void 
bit_statistics::inc_invalid(int n){
  nInvalid+=n;
}
#endif
