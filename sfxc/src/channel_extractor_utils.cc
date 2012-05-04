/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - sfxc/SCARIe
 * This file contains:
 *   - few utils used by the several of the channel_extractor I
 *     have developped.
 */
#include <vector>
#include <iostream>
#include "channel_extractor_utils.h"


int compute_span_out(const std::vector< std::vector<int> > &track_positions) {
  //int span_out=0;
  //unsigned int cpt = 0;
  unsigned int mask[16];
  for (unsigned int i=0;i<track_positions.size(); i++)
    mask[i] = 0;

  for (unsigned int i=0;i<track_positions.size(); i++) {
    for (unsigned int j=0;j<track_positions[i].size();j++) {
      if ( j == 0 && track_positions[i][j] < 8 && (mask[i]) == 0 ) {
        mask[i]++;
      }
    }
  }

  int toto=0;
  for (unsigned int i=0;i<track_positions.size();i++) {
    toto += mask[i];
  }

  //std::cout << "TOTO "<< toto;
  return toto;
}
