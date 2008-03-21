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
 *   - Implementation of a channel extractor with look up table
 *     faster than "number 5" but less flexible.
 */
#include <cassert>
#include <iostream>
#include "channel_extractor_fast.h"
#include "channel_extractor_utils.h"

//template<>
template<
int Tsize_input_word,
int Tn_subbands,
int Tfan_out,
int Tsamples_per_byte,
int Tseqsize>
class Channel_extractor_fast_impl : public Channel_extractor_interface {
public:
  Channel_extractor_fast_impl() {}

  void initialise(const std::vector< std::vector<int> > &track_positions,
                  int IGNORED_size_of_one_input_word,
                  int input_sample_size) {

  }

  virtual void extract(unsigned char *in_data1,
                       unsigned char **output_data) {}


  void initialise(const std::vector< std::vector<int> > &track_positions) {
    assert( Tn_subbands == track_positions.size() );
    assert( Tfan_out == track_positions[0].size() );
    assert(Tfan_out <= 8);
    assert(8%Tfan_out == 0);
    assert(Tfan_out*Tsamples_per_byte == 8);

    std::cout << "Table builder: " << std::endl;
    std::cout << "     Subbands: " << Tn_subbands << std::endl;
    std::cout << "      fan out: " << Tfan_out << std::endl;
    std::cout << " samples/byte: " << Tsamples_per_byte << std::endl;
    std::cout << "sequence size: " << Tseqsize << std::endl;

    std::cout << "Table builder: " << std::endl;
    for (unsigned int i=0;i<track_positions.size();i++) {
      std::cout << "Channel "<<i<<": ";
      for (unsigned int j=0;j<track_positions[i].size();j++) {
        std::cout << track_positions[i][j] << ", ";
      }
      std::cout << std::endl;
    }

    table = new std::vector<Action>**[Tseqsize];
    //ftable = new uint8_t**[seqsize];
    for (unsigned int i=0;i<Tseqsize;i++) {
      table[i] = new std::vector<Action>*[256];
      for (unsigned int j=0;j<256;j++) {
        table[i][j] = new std::vector<Action>();
      }
    }

    int seqpref = 0;
    int cshift[256][Tn_subbands];
    for (unsigned int i=0;i<256;i++)
      for (int j=0;j<Tn_subbands;j++)
        cshift[i][j] = 7;

    for (unsigned int r=0;r<Tseqsize;r+=Tfan_out) {
      for (unsigned int i=0;i<Tn_subbands;i++) {
        //std::cout << "Doing TABLE for channel "<<i<<": ";
        for (unsigned int j=0;j<track_positions[i].size();j++) {
          uint8_t idx = track_positions[i][j];
          uint8_t seqidx = r  + idx / 8;
          uint8_t cidx = idx % 8;
          //std::cout << "SHIFING("<< (int)r << ":"<< (int)seqidx <<","<< (int)cidx <<"): " << std::endl;
          for (unsigned int k=0;k<256;k++) {
            find_add( table[seqidx][k], i, (k>>cidx&1) << (cshift[k][i]), (cshift[k][i]) );
            (cshift[k][i])--;
          }
          //std::cout << std::endl;
          //std::cout << ", " ;
        }
        //std::cout << std::endl;
      }
    }

    int tidx=0;
    for (unsigned int i=0;i<256;i++) {
      for (unsigned int j=0;j<Tseqsize;j++) {
        std::vector<Action>::iterator bg = (*table[j][i]).begin();
        std::vector<Action>::iterator en = (*table[j][i]).end();

        int idefix=0;
        //std::cout << "[";
        while ( bg != en ) {
          newtab[i][j][idefix] = ((unsigned char)(*bg).value) ;
          idefix+=1;
          if (i == 255 ) {
            std::cout << (*bg).channel << ":("<< (*bg).value << ") ";
            order[tidx] = (*bg).channel;
            std::cout << "Adding: " << tidx << " " << (*bg).channel+1 <<" " << std::endl;

            tidx++;
          }
          bg++;

        }
      }
    }

    for (unsigned int i=0; i<Tseqsize*Tsamples_per_byte;i++) {
      fasttmp[i] = &(tmpout[ order[i] ] );
    }
    memset(tmpout, 0, Tn_subbands+1);
  }

  void do_task(int n_input_samples,
               int offset,
               const char in_data[],
               std::vector<char *> output_data) {
    switch (offset) {
    case 0:
      do_task_no_offset(n_input_samples, in_data, output_data);
      break;
    case 1:
      do_task_offset<1, 7>(n_input_samples, in_data, output_data);
      break;
    case 2:
      do_task_offset<2, 6>(n_input_samples, in_data, output_data);
      break;
    case 3:
      do_task_offset<3, 5>(n_input_samples, in_data, output_data);
      break;
    default:
      assert("Strange offset for this fan_out");
    }
  }



  template<int Toffset, int Tshift>
  void do_task_offset(int n_input_samples,
                      const char in_data[],
                      std::vector<char *> output_data) {

    unsigned int outindex=0;
    unsigned char* currbuffer = (unsigned char*)in_data;
    unsigned char* endbuffer = currbuffer+(n_input_samples+1)*Tsize_input_word;

    //uint8_t* dorder = order;
    //char** cfasttmp = &(fasttmp[0]);
    //int shift = 8-offset;
    //shift = 8;
    for (unsigned int i=0;i<Tseqsize;++i) {
      char* dst;
      unsigned char* channelidxval2;

      channelidxval2 = &(newtab[(*currbuffer)][i][0]);
      dst = fasttmp[i*Tsamples_per_byte];

      for (unsigned int j=0;j<Tsamples_per_byte;j++) {
        //tmpout[ dorder[j] ] |= (*(channelidxval2+j));
        *(dst+j) |= (*(channelidxval2+j));
      }
      //dorder+=Tsamples_per_byte;
      //cfasttmp+=Tsamples_per_byte;
      ++currbuffer;
    }
    //cfasttmp = fasttmp;
    //dorder = order;

    // the sequence is terminated...
    for (unsigned int i=0;i<Tn_subbands;i++) {
      output_data[i][outindex] = tmpout[i];
      tmpout[i]=0;
    }
    ++outindex;

    while (currbuffer < endbuffer) {
      for (unsigned int i=0;i<Tseqsize;++i) {
        char* dst;
        unsigned char* channelidxval2;

        channelidxval2 = &(newtab[(*currbuffer)][i][0]);
        dst = fasttmp[i*Tsamples_per_byte];

        for (unsigned int j=0;j<Tsamples_per_byte;j++) {
          //tmpout[ dorder[j] ] |= (*(channelidxval2+j));
          *(dst+j) |= (*(channelidxval2+j));
        }
        //dorder+=Tsamples_per_byte;
        //cfasttmp+=Tsamples_per_byte;
        ++currbuffer;
      }
      //cfasttmp = fasttmp;
      //dorder = order;

      // the sequence is terminated...
      for (unsigned int i=0;i<Tn_subbands;i++) {
        output_data[i][outindex-1] = (output_data[i][outindex-1] << Toffset) | (((unsigned char)tmpout[i])>> Tshift);
        output_data[i][outindex] = tmpout[i];
        tmpout[i]=0;
      }
      ++outindex;
    }

    //for (unsigned int i=0;i<Tn_subbands;i++)
    //{
    //output_data[i][outindex-1] = (output_data[i][outindex-1] << Toffset) | (((unsigned char)tmpout[i])>>Tshift);
    //}
  }

  void do_task_no_offset(int n_input_samples,
                         const char in_data[],
                         std::vector<char *> output_data) {

    unsigned int outindex=0;
    unsigned char* currbuffer = (unsigned char*)in_data;
    unsigned char* endbuffer = currbuffer+(n_input_samples)*Tsize_input_word;

    while (currbuffer < endbuffer) {
      for (unsigned int i=0;i<Tseqsize;++i) {
        char* dst;
        unsigned char* channelidxval2;

        channelidxval2 = &(newtab[(*currbuffer)][i][0]);
        dst = fasttmp[i*Tsamples_per_byte];

        for (unsigned int j=0;j<Tsamples_per_byte;j++) {
          *(dst+j) |= (*(channelidxval2+j));
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

  uint8_t  order[Tseqsize*Tsamples_per_byte];
  char* fasttmp[Tseqsize*Tsamples_per_byte];
  uint8_t  newtab[256][Tseqsize][Tsamples_per_byte];

  char tmpout[Tn_subbands+1];
};

Channel_extractor_fast::Channel_extractor_fast() {
  name_ = "Channel_extractor_fast (Damien love optimization !)";
  hidden_implementation_ = NULL;
}

template<int Tsize_input_word, int Tfan_out>
Channel_extractor_fast* create(int input_sample_size_) {
  return NULL; // create<Tsize_input_word, Tfan_out>(input_sample_size_);
}

template<int Tsize_input_word>
Channel_extractor_fast* create(int fan_out, int input_sample_size_) {
  switch (fan_out) {
  case 1:
    return create<Tsize_input_word, 1>(input_sample_size_);
  case 2:
    return create<Tsize_input_word, 2>(input_sample_size_);
  case 4:
    return create<Tsize_input_word, 4>(input_sample_size_);
  case 8:
    return create<Tsize_input_word, 8>(input_sample_size_);
  default:
    return NULL;
  }
}

void Channel_extractor_fast::initialise(const std::vector< std::vector<int> > &track_positions_,
                                        int size_of_one_input_word_,
                                        int input_sample_size_) {
  track_positions = track_positions_;
  size_of_one_input_word = size_of_one_input_word_;
  input_sample_size = input_sample_size_+1;
  fan_out = track_positions[0].size();
  n_subbands = track_positions.size();

  switch (size_of_one_input_word) {
  case 1:
    hidden_implementation_ = create<1>(fan_out, input_sample_size);
    break;
  case 2:
    hidden_implementation_ = create<2>(fan_out, input_sample_size);
    break;
  case 4:
    hidden_implementation_ = create<4>(fan_out, input_sample_size);
    break;
  case 8:
    hidden_implementation_ = create<8>(fan_out, input_sample_size);
    break;
  default:
    std::cerr << " size_of_input_word: " << size_of_one_input_word << std::endl;
    assert(false&&"Unsupported size of input word for channelization");
  }
  hidden_implementation_->initialise(track_positions, size_of_one_input_word_, input_sample_size);
}

void Channel_extractor_fast::extract(unsigned char *in_data1,
                                     unsigned char *in_data2,
                                     int samples_in_data1, /* <= size_of_one_input_word+1 */
                                     unsigned char **output_data) {
  assert( hidden_implementation_ != NULL && "big" );

  hidden_implementation_->extract(in_data1, output_data);
}

