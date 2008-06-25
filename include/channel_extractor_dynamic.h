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
 *   - declaration of the Channel_extrator_dynamic class.
 */
#ifndef CHANNEL_EXTRACTOR_DYNAMIC_H__
#define CHANNEL_EXTRACTOR_DYNAMIC_H__

#include "channel_extractor_interface.h"

/*******************************************************************************
*
* @class Channel_extractor_dynamic
* @desc The fastest channel extractor.
* This extractor can in fact load from a specific file a pre-compiled
* channel_extractor specific to the data patern that have to be treated.
* It can also dynamically (during initialization) generate and compile
* the needed extractor (this require a compile).
*******************************************************************************/

#include <string>
#include "channel_extractor_interface.h"

class Channel_extractor_dynamic : public Channel_extractor_interface {
    public:
      Channel_extractor_dynamic(const std::string& dirname="./", bool compile=false);

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

      // Location (directory) where the optimized channel extractor are stored.
      std::string dirname_;

      // Indicate of we have to compile the missing extracted.
      bool compile_;

      Channel_extractor_interface* dlload_and_construct(const std::string& name);

      Channel_extractor_interface* compile_load_construct(
        const std::string& filename,
        const std::vector< std::vector<int> > &track_positions_,
        int size_of_one_input_word_,
        int input_sample_size_);
    };
#endif // CHANNEL_EXTRACTOR_DYNAMIC_H__

