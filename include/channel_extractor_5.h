/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands) * Copyright (c) 2007 University of Amsterdam (Netherlands) * All rights reserved. * * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007 *            Damien Marchal <dmarchal@science.uva.nl>, 2007 * * * This file is part of: *   - sfxc/SCARIe * This file contains: *   - An implementation of faster than *     channel_extractor_brute_force. *     this method use precomputation table. */#ifndef CHANNEL_EXTRACTOR_5_H__#define CHANNEL_EXTRACTOR_5_H__#include "channel_extractor_interface.h"class Channel_extractor_5 : public Channel_extractor_interface {public:  Channel_extractor_5();

  void initialise(const std::vector< std::vector<int> > &track_positions_,
                  int size_of_one_input_word_,
                  int input_sample_size_);

  void extract(unsigned char *in_data1,
               unsigned char **output_data);
private:
  Channel_extractor_interface* hidden_implementation_;

  std::vector< std::vector<int> > track_positions;
  int size_of_one_input_word;
  int input_sample_size;
  int n_subbands;

  // Computed
  int fan_out;
};

#endif // CHANNEL_EXTRACTOR_5_H__
