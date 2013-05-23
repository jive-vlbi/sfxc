/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema, <keimpema@jive.nl>
 *
 */

#include <math.h>
#include <string.h>
#include "bit_statistics.h"

bit_statistics::bit_statistics():bits_per_sample(-1){
  data_counts.resize(256);
  memset(&data_counts[0],0,data_counts.size()*sizeof(int));
  nInvalid = 0;
}

bit_statistics::~bit_statistics(){
}

void 
bit_statistics::reset_statistics(int bits_per_sample_, int scale_){
  SFXC_ASSERT(bits_per_sample_ == 1 || bits_per_sample_ == 2);
  if(bits_per_sample != bits_per_sample_){
    bits_per_sample = bits_per_sample_;
#if 0
    statistics.resize((int)pow(2,bits_per_sample)+1); // +1 for the #invalid samples
#else
    statistics.resize(5);
#endif
  }
  scale = scale_;
  memset(&data_counts[0],0,data_counts.size()*sizeof(int));
  nInvalid = 0;
}

int *
bit_statistics::get_statistics(){
  SFXC_ASSERT((bits_per_sample>=1)&&(bits_per_sample<=8))
  memset(&statistics[0],0,statistics.size()*sizeof(int));

  if(bits_per_sample == 2){
    for(int i=0;i<256;i++){
      statistics[i&3] += data_counts[i];
      statistics[(i>>2)&3] += data_counts[i];
      statistics[(i>>4)&3] += data_counts[i];
      statistics[(i>>6)&3] += data_counts[i];
    }
  }else{
    int bps = bits_per_sample;
#if 0
    int max_val = statistics.size()-2;
#else
    int max_val = 1;
#endif
    for(int i=0;i<256;i++){
      for(int j=0;j<8/bps;j++)
        statistics[(i>>j*bps)&max_val] += data_counts[i];
    }
  }
  statistics[statistics.size()-1] += nInvalid;
  for (size_t i = 0; i < statistics.size(); i++)
    statistics[i] /= scale;
  return &statistics[0];
}
