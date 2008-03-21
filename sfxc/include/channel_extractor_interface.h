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
 *   - The declaration of a class that have to
 *     implement in order to provide channel_extraction
 *     into scarie.
 */

#ifndef CHANNEL_EXTRACTOR_INTERFACE_H
#define CHANNEL_EXTRACTOR_INTERFACE_H

#include <vector>
#include <string>

class Channel_extractor_interface {
protected:
  std::string name_;
public:
  Channel_extractor_interface() { name_="Base_extractor_interface"; }
  virtual ~Channel_extractor_interface() {}

  // Size of one input word to process in the extract function
  // Track position array
  // size of one input sample in bytes
  virtual void initialise(const std::vector< std::vector<int> > &track_positions,
                          int size_of_one_input_word,
                          int input_sample_size) = 0;

  // extract samples 0..(samples_in_data1-1) from data1
  // extract samples samples_in_data1..size_of_one_input_word from data2
  // in_data1 is not aligned with the sequence number

  // #define MAX_NR_SUBBANDS 32
  // nr_subbands (for the output) = track_positions.size()
  // fan_out = track_positions[0].size() == track_positions[i].size()
  // fan_out is the number of output bits per input sample
  // fan_out is 1 2 4 8
  // offset is 0 .. fan_out-1
  virtual void extract(unsigned char *in_data1,
                       unsigned char **output_data) = 0;

  std::string& name(){ return name_; }
};

#endif // CHANNEL_EXTRACTOR_INTERFACE_H
