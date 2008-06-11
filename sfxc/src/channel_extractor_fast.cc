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
 *   - Implementation of a channel extractor with look up table.
 *     this implementation is faster than "number 5" but less
 *     flexible. Don't be afraid by the compile time as the code
 *     is highly templated to help the compile to detect the constant
 *     (number of channels, fan_out, etc..). The performance dependon
 *     the bit pattern. With the worst bit pattern the performance on
 *     an IntelCore2 is from [60MB/s (16 channel) to 130MB/s].
 *     On DAS3 cut the number by 2 to get a rule of thumb approximation.
 */
#include "utils.h"
#include <iostream>
#include "channel_extractor_fast.h"
#include "channel_extractor_utils.h"


// We use a lot of template to enforce all of this
// to be consider constant by the compiler
// We also use an hidden-class to hide these details to the
// user of the channel_extractor.
template<
int Tsize_input_word,
int Tn_subbands,
int Tfan_out,
int Tsamples_per_byte,
int Tseqsize,
int Tspan_out
>
class Channel_extractor_fast_impl : public Channel_extractor_interface {
public:
  Channel_extractor_fast_impl() {}

  void initialise(const std::vector< std::vector<int> > &track_positions,
                  int IGNORED_size_of_one_input_word,
                  int input_sample_size) {
    input_sample_size_ = input_sample_size;
    initialise( track_positions );
  }

  /*************************************************************************
  *
  *  the input stream is cutted in sequence. A sequence is the number of
  *  input byte needed to generate one output byte on each of the channel.
  *  the precomputed table contains the following:
  *  newtable[input_byte_value][sequence_index][span_out_index] = value
  *
  *  The span_out_index is < span_out. Span_out is the number of different
  *  channels that are 'touched' by one input byte.
  *
  *  A second table named fasttmp stores the destination_channel for each
  *  sequence_index*span_out+span_out_index
  *
  *  while currbyteptr < totalbyteptr:
  *    for i in sequence_index:
  *        for j in span_out_index:
  *          fasttmp |= newtable[*currbyteptr][i][j]
  *    copy temporary result to real output
  *    currbyteptr+=sequence_size
  */
  void initialise(const std::vector< std::vector<int> > &track_positions) {
    SFXC_ASSERT( Tn_subbands == track_positions.size() );
    SFXC_ASSERT( Tfan_out == track_positions[0].size() );
    SFXC_ASSERT(Tfan_out <= 8);
    SFXC_ASSERT(8%Tfan_out == 0);
    SFXC_ASSERT(Tfan_out*Tsamples_per_byte == 8);
    verbose = false;
    if (verbose) {

      std::cout << "Table builder: " << std::endl;
      std::cout << "     Subbands: " << Tn_subbands << std::endl;
      std::cout << "      fan out: " << Tfan_out  << std::endl;
      std::cout << " samples/byte: " << Tsamples_per_byte << std::endl;
      std::cout << "sequence size: " << Tseqsize  << std::endl;
      std::cout << "     span_out: " << Tspan_out << std::endl;
      std::cout << "size of input word:" << Tsize_input_word << std::endl;
      std::cout << "Table builder: " << std::endl;

      for (unsigned int i=0;i<track_positions.size();i++) {
        std::cout << "Channel "<<i<<": ";
        for (unsigned int j=0;j<track_positions[i].size();j++) {
          std::cout << track_positions[i][j] << ", ";
        }
        std::cout << std::endl;
      }
    }
    table = new std::vector<Action>**[Tseqsize];
    //ftable = new uint8_t**[seqsize];
    for (unsigned int i=0;i<Tseqsize;i++) {
      table[i] = new std::vector<Action>*[256];
      for (unsigned int j=0;j<256;j++) {
        table[i][j] = new std::vector<Action>(0,0);
      }
    }

    //int seqpref = 0;
    int cshift[256][Tn_subbands];
    for (unsigned int i=0;i<256;i++)
      for (int j=0;j<Tn_subbands;j++)
        cshift[i][j] = 7;

    for (unsigned int r=0;r<Tseqsize;r+=Tsize_input_word) {
      for (unsigned int i=0;i<Tn_subbands;i++) {
        for (unsigned int j=0;j<track_positions[i].size();j++) {
          uint8_t idx = track_positions[i][j];
          uint8_t seqidx = r + (idx/8);
          uint8_t cidx = (idx) % 8;
          for (unsigned int k=0;k<256;k++) {
            find_add( table[seqidx][k], i, ((k>>cidx)&1)<<cshift[k][i], (cshift[k][i]) );
            (cshift[k][i])--;
          }
        }
      }
    }
    int tidx=0;
    for (unsigned int i=0;i<256;i++) {
      for (unsigned int j=0;j<Tseqsize;j++) {
        std::vector<Action>::iterator bg = (*table[j][i]).begin();
        std::vector<Action>::iterator en = (*table[j][i]).end();
        int idefix=0;
        while ( bg != en ) {
          SFXC_ASSERT( idefix < Tspan_out);
          newtab[i][j][idefix] = ((unsigned char)(*bg).value);
          idefix+=1;
          if (i == 255 ) {
            SFXC_ASSERT( tidx < Tseqsize*Tspan_out);
            order[tidx] = (*bg).channel;
            tidx++;
          }
          bg++;
        }
      }
    }
    for (unsigned int i=0; i<Tseqsize*Tspan_out;i++) {
      fasttmp[i] = &(tmpout[ order[i] ] );
    }
    memset(tmpout, 0, Tn_subbands+1);
  }

  virtual void extract(unsigned char *in_data1, unsigned char **output_data) {
    do_task_no_offset( input_sample_size_, in_data1, output_data );
  }

  void do_task_no_offset(int n_input_samples,
                         const unsigned char * in_data,
                         unsigned char ** output_data) {

    unsigned int outindex=0;
    unsigned char* currbuffer = (unsigned char*)in_data;
    unsigned char* endbuffer = currbuffer+(n_input_samples)*Tsize_input_word;

    while (currbuffer < endbuffer) {
      for (unsigned int i=0;i<Tseqsize;++i) {
        unsigned char** dst;
        unsigned char* channelidxval2;

        channelidxval2 = &(newtab[(*currbuffer)][i][0]);
        dst = &(fasttmp[i*Tspan_out]);

        for (unsigned int j=0;j<Tspan_out;j++) {
          (**(dst+j)) |= (*(channelidxval2+j));
        }
        ++currbuffer;
      }
      for (unsigned int i=0;i<Tn_subbands;i++) {
        output_data[i][outindex] = tmpout[i];
        tmpout[i]=0;
      }

      ++outindex;
    }
  }
private:

private:
  std::vector<Action> ***table;

  uint8_t  order[Tseqsize*Tspan_out];
  unsigned char* fasttmp[Tseqsize*Tspan_out];
  uint8_t  newtab[256][Tseqsize][Tspan_out];

  unsigned char tmpout[Tn_subbands+1];
  int input_sample_size_;

  bool verbose;
};


Channel_extractor_fast::Channel_extractor_fast() {
  name_ = "Channel_extractor_fast";
  hidden_implementation_ = NULL;
}

// Create an extractor by converting the parameters
// as template arguments
template<int Tsize_of_one_input_word, int Tfan_out, int Tn_subbands, int Tspan_out>
Channel_extractor_interface* create_fast_(int input_sample_size) {
  return new Channel_extractor_fast_impl<Tsize_of_one_input_word, Tn_subbands, Tfan_out, 8/Tfan_out, Tsize_of_one_input_word*8/Tfan_out, Tspan_out>();
}

// Create an extractor by converting the parameters
// as template arguments
template<int Tsize_of_one_input_word, int Tfan_out, int Tn_subbands>
Channel_extractor_interface* create_fast_(int input_sample_size, int span_out) {
  switch (span_out) {
  case 1:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, Tn_subbands, 1>(input_sample_size);
  case 2:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, Tn_subbands, 2>(input_sample_size);
  case 4:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, Tn_subbands, 4>(input_sample_size);
  case 8:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, Tn_subbands, 8>(input_sample_size);
  default:
    std::cerr << " invalid span_out for channel_extractor_fast: " << span_out << std::endl;
    SFXC_ASSERT(false&&"Unsupported span_out for channel_extractor_fast");
  }
  return NULL;
}


// Create an extractor by converting the parameters
// as template arguments
template<int Tsize_of_one_input_word, int Tfan_out>
Channel_extractor_interface* create_fast_(int n_subbands, int input_sample_size, int span_out) {
  switch (n_subbands) {
  case 1:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, 1>(input_sample_size, span_out);
  case 2:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, 2>(input_sample_size, span_out);
  case 4:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, 4>(input_sample_size, span_out);
  case 8:
    return create_fast_<Tsize_of_one_input_word, Tfan_out, 8>(input_sample_size, span_out);
  case 16:
    return create_fast_<Tsize_of_one_input_word, Tfan_out,16>(input_sample_size, span_out);
  default:
    std::cerr << " invalid number of subbands for channel_extractor_fast: " << n_subbands << std::endl;
    SFXC_ASSERT(false&&"Unsupported number of subbands for channel_extractor_fast");
  }
  return NULL;
}

// Create an extractor by converting the parameters
// as template arguments
template<int Tsize_of_one_input_word>
Channel_extractor_interface* create_fast_(int fan_out,
    int n_subbands, int input_sample_size, int span_out) {
  switch (fan_out) {
  case 1:
    return create_fast_<Tsize_of_one_input_word,1>(n_subbands, input_sample_size, span_out);
  case 2:
    return create_fast_<Tsize_of_one_input_word,2>(n_subbands, input_sample_size, span_out);
  case 4:
    return create_fast_<Tsize_of_one_input_word,4>(n_subbands, input_sample_size, span_out);
  case 8:
    return create_fast_<Tsize_of_one_input_word,8>(n_subbands, input_sample_size, span_out);
  default:
    std::cerr << " invalid fan_out for channel_extractor_fast: " << fan_out << std::endl;
    SFXC_ASSERT(false&&"Unsupported fan_out for channelization");
  }
  return NULL;
}

// Create an extractor by converting the parameters
// as template arguments
Channel_extractor_interface* create_fast_(int size_of_one_input_word, int fan_out,
    int n_subbands, int input_sample_size, int span_out) {
  switch (size_of_one_input_word) {
  case 1:
    return create_fast_<1>(fan_out, n_subbands, input_sample_size, span_out);
  case 2:
    return create_fast_<2>(fan_out, n_subbands, input_sample_size, span_out);
  case 4:
    return create_fast_<4>(fan_out, n_subbands, input_sample_size, span_out);
  case 8:
    return create_fast_<8>(fan_out, n_subbands, input_sample_size, span_out);
  default:
    std::cerr << " invalid size_of_input_word for channel_extractor_fast: " << size_of_one_input_word << std::endl;
    SFXC_ASSERT(false&&"Unsupported size of input word for channelization");
  }
  return NULL;
}


void Channel_extractor_fast::initialise(const std::vector< std::vector<int> > &track_positions_,
                                        int size_of_one_input_word_,
                                        int input_sample_size_) {
  track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_+1;
  fan_out = track_positions[0].size();
  n_subbands = track_positions.size();

  int span_out = compute_span_out( track_positions_ );

  hidden_implementation_ = create_fast_(size_of_one_input_word, fan_out, n_subbands, input_sample_size, span_out);
  if ( hidden_implementation_ == NULL ) {
    std::cerr << " No channel_extractor can be build" << std::endl;
    SFXC_ASSERT(false && "No channel extractor can be build with these parameters");
  }
  hidden_implementation_->initialise(track_positions, size_of_one_input_word_, input_sample_size);
}

void Channel_extractor_fast::extract(unsigned char *in_data1,
                                     unsigned char **output_data) {
  SFXC_ASSERT( hidden_implementation_ != NULL && " NULL pointer usage" );
  hidden_implementation_->extract(in_data1, output_data);
}


