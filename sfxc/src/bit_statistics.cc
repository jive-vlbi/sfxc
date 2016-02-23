/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema, <keimpema@jive.nl>
 *
 */

#include <math.h>
#include <string.h>
#include "bit_statistics.h"

bit_statistics::bit_statistics() : bits_per_sample(-1) {
  data_counts_on.assign(256, 0);
  data_counts_off.assign(256, 0);
  nInvalid = 0;
}

bit_statistics::~bit_statistics() {
}

void 
bit_statistics::reset_statistics(int bits_per_sample_, uint64_t sample_rate_,
				 uint64_t base_sample_rate_) {
  SFXC_ASSERT(bits_per_sample_ == 1 || bits_per_sample_ == 2);
  bits_per_sample = bits_per_sample_;
  sample_rate = sample_rate_;
  base_sample_rate = base_sample_rate_;
  data_counts_on.assign(256, 0);
  data_counts_off.assign(256, 0);
  nInvalid = 0;
}

int *
bit_statistics::get_statistics() {
  SFXC_ASSERT((bits_per_sample >= 1) && (bits_per_sample <= 8));
  statistics.assign(5, 0);

  if (bits_per_sample == 2) {
    for (int i = 0; i < 256; i++) {
      statistics[i & 3] += data_counts_on[i];
      statistics[(i >> 2) & 3] += data_counts_on[i];
      statistics[(i >> 4) & 3] += data_counts_on[i];
      statistics[(i >> 6) & 3] += data_counts_on[i];
      statistics[i & 3] += data_counts_off[i];
      statistics[(i >> 2) & 3] += data_counts_off[i];
      statistics[(i >> 4) & 3] += data_counts_off[i];
      statistics[(i >> 6) & 3] += data_counts_off[i];
    }
  } else {
    int bps = bits_per_sample;
    int max_val = ((1 << bps) - 1);
    for(int i = 0; i < 256; i++) {
      for(int j = 0; j < (8 /bps); j++) {
        statistics[(i >> j * bps) & max_val] += data_counts_on[i];
        statistics[(i >> j * bps) & max_val] += data_counts_off[i];
      }
    }
  }
  statistics[statistics.size()-1] += nInvalid;
  for (size_t i = 0; i < statistics.size(); i++)
    statistics[i] = (base_sample_rate * statistics[i]) / sample_rate;
  return &statistics[0];
}

int *
bit_statistics::get_tsys() {
  std::vector<int> on, off;

  on.assign(4, 0);
  off.assign(4, 0);
  tsys.assign(4, 0);

  if (bits_per_sample == 2) {
    for (int i = 0; i < 256; i++) {
      on[i & 3] += data_counts_on[i];
      on[(i >> 2) & 3] += data_counts_on[i];
      on[(i >> 4) & 3] += data_counts_on[i];
      on[(i >> 6) & 3] += data_counts_on[i];
      off[i & 3] += data_counts_off[i];
      off[(i >> 2) & 3] += data_counts_off[i];
      off[(i >> 4) & 3] += data_counts_off[i];
      off[(i >> 6) & 3] += data_counts_off[i];
    }
    tsys[0] = on[1] + on[2];
    tsys[1] = on[0] + on[3];
    tsys[2] = off[1] + off[2];
    tsys[3] = off[0] + off[3];
  }

  return &tsys[0];
}
