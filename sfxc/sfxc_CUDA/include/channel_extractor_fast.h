/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - SFXC/SCARIe project.
 * This file contains:
 *   - declaration of the Channel_extrator_fast class.
 */
#ifndef CHANNEL_EXTRACTOR_FAST_H__
#define CHANNEL_EXTRACTOR_FAST_H__

#include "channel_extractor_interface.h"

/*******************************************************************************
*
* @class Channel_extractor_fast
* @desc The fastest static channel extractor.
* The performance dependon
* the bit pattern. With the worst bit pattern the performance on
* an IntelCore2 is from [60MB/s (16 channel) to 130MB/s].
* On DAS3 cut the number by 2 to get a rule of thumb approximation. Don't
* be worried by the compile time. Everything that is done at compiled time...
* will not to be done at run time :)
*******************************************************************************/
class Channel_extractor_fast : public Channel_extractor_interface {
public:
  Channel_extractor_fast();

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

#endif // CHANNEL_EXTRACTOR_FAST_H__

