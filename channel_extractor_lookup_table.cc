#include <iostream>
#include <vector>
#include <assert.h>

#include "channel_extractor_interface.h"
#include "channel_extractor1.h"

#include "timer.h"

#define VERBOSE


typedef uint32_t Type;
const int N = sizeof(Type);
#define TN sizeof(Type)

template <class Type>
std::ostream &print_hex(std::ostream &out, const Type &t) {
#ifdef VERBOSE
#if 0
  out << std::hex;
  for (int i=2*sizeof(Type)-1; i>=0; i--) {
    out << ((t>>(4*i))&15);
  }
  out << std::dec;
#else

  for (int i=8*sizeof(Type)-1; i>=0; i--) {
    out << ((t>>i)&1);
    if ((i>0) && (i%8==0))
      out << ".";
  }
#endif
#endif // VERBOSE
  return out;
}

class Extractor
{
	public:
		virtual void do_task(int n_input_samples,
        	       int offset,
        	       const Type in_data[],
        	       std::vector<char *> output_data) = 0;
};



class Extractor1 {
public:
  Extractor1(const std::vector< std::vector<int> > &track_positions)
      : tracks_in_subbands(track_positions) {}

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {
    int n_tracks_per_subband = tracks_in_subbands[0].size();
    int bit_position=0, byte_position=0;

    process_samples(&in_data[0],
                    1,
                    offset, n_tracks_per_subband,
                    output_data,
                    bit_position, byte_position);
    process_samples(&in_data[1],
                    n_input_samples-1,
                    0, n_tracks_per_subband,
                    output_data,
                    bit_position, byte_position);
    process_samples(&in_data[n_input_samples],
                    1,
                    0, offset,
                    output_data,
                    bit_position, byte_position);
  }
  void process_samples(const Type *input_buffer,
                       int n_input_samples,
                       int start_track_nr,
                       int end_track_nr,
                       std::vector<char *> &output_buffer,
                       int &output_buffer_bit,
                       int &output_buffer_byte) {
    // for each input sample
    for (int input_sample = 0; input_sample < n_input_samples; input_sample++) {
      // for each track in the output subband
      for (int track_nr = start_track_nr; track_nr < end_track_nr; track_nr++) {
        // for each subband
        for (int subband=(int)output_buffer.size()-1; subband >= 0 ; subband--) {
          //for (size_t subband=0; subband < output_buffer.size() ; subband++) {
          // bit shift
          output_buffer[subband][output_buffer_byte] =
            (output_buffer[subband][output_buffer_byte]<<1);
          // add new sample
          output_buffer[subband][output_buffer_byte] +=
            (((*input_buffer) >> tracks_in_subbands[subband][track_nr]) & 1);
        }

        // go to the next bit
        output_buffer_bit ++;
        if (output_buffer_bit==8) { // Compute modulo 8
          output_buffer_bit = 0;
          output_buffer_byte ++;
        }
      }
      input_buffer++;
    }
  }
private:
  std::vector< std::vector<int> > tracks_in_subbands;
};

class Extractor2 {
public:
  Extractor2(const std::vector< std::vector<int> > &track_positions) {
    // bit shift right with padding
    for (int shift=0; shift<8; shift++) {
      for (int value=0; value<256; value++) {
        bit_shift_right_table[shift][value] =
          (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
      }
    }

    n_subbands = track_positions.size();
    fan_out = track_positions[0].size();
    assert(fan_out <= 8);
    assert(8%fan_out == 0);

    memset(lookup_table, 0, N*256*8);
    // Lookup table for the tracks
    for (int subband=0; subband < n_subbands; subband++) {
      for (int track_nr=0; track_nr < fan_out; track_nr++) {
        int track = track_positions[subband][track_nr];
        int n = track/8;
        assert(n < N);
        int bit = track % 8;
        for (int sample=0; sample<256; sample++) {
          if (((sample >> bit)& 1) != 0) {
            lookup_table[n][sample][subband] |= (1<<(fan_out-1-track_nr));
          }
        }
      }
    }
  }

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {
    assert((int)output_data.size() == n_subbands);
    char *output_[output_data.size()];
    for (size_t i=0; i<output_data.size(); i++) {
      output_[i] = output_data[i];
    }
    const uint8_t *in_array = (uint8_t *)&in_data[0];

    int subsample = 0;
    int bit = 0;
    for (int subband=0; subband<n_subbands; subband++) {
      memset((void *)output_data[subband], 0, n_input_samples*fan_out/8+1);
    }
    for (int sample=0; sample<n_input_samples+1; sample++) {
      for (int n=0; n<N; n++) {
        uint8_t *in_sample = lookup_table[n][in_array[subsample]];

        for (int subband=0; subband<n_subbands; subband++) {
          *output_data[subband] |= *in_sample;
          in_sample++;
        }

        subsample++;
      }

      // Shift the samples
      bit += fan_out;
      if (bit == 8) {
        for (int subband=0; subband<n_subbands; subband++)
          output_data[subband]++;
        bit = 0;
      } else {
        for (int subband=0; subband<n_subbands; subband++)
          output_data[subband][0] = (output_data[subband][0] << fan_out);
      }
    }

    { // Do the offset
      uint8_t *mask_right = bit_shift_right_table[offset];
      for (int subband=0; subband<n_subbands; subband++) {
        // shift the last sample
        if (fan_out < 4)
          output_data[subband][0] = (output_data[subband][0]<<(8-2*fan_out));

        char *data = output_[subband];
        for (int sample=0; sample<n_input_samples*fan_out/8; sample++) {
          *data = (*data << offset) | mask_right[(uint8_t)*(data+1)];
          data++;
        }
      }
    }
  }

private:
  // Lookup table for the channel extraction
  // At most 8 channels
  // lookup_table[index in Type word][value of the byte][output sample per channel]
  uint8_t lookup_table[N][256][8];

  // Lookup table for a right bitshift with 0 inserted
  uint8_t bit_shift_right_table[8][256];

  // Fan out
  int fan_out;

  int n_subbands;
};


class Extractor3 {
public:
  Extractor3(const std::vector< std::vector<int> > &track_positions) {
    // bit shift right with padding
    for (int shift=0; shift<8; shift++) {
      for (int value=0; value<256; value++) {
        bit_shift_right_table[shift][value] =
          (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
      }
    }

    n_subbands = track_positions.size();
    fan_out = track_positions[0].size();
    samples_per_byte = 8/fan_out;
    assert(fan_out <= 8);
    assert(8%fan_out == 0);
    assert(fan_out*samples_per_byte == 8);

    memset(lookup_table, 0, N*256*8);
    // Lookup table for the tracks
    for (int subband=0; subband < n_subbands; subband++) {
      for (int track_nr=0; track_nr < fan_out; track_nr++) {
        int track = track_positions[subband][track_nr];
        int n = track/8;
        assert(n < N);
        int bit = track % 8;
        for (int sample=0; sample<256; sample++) {
          if (((sample >> bit)& 1) != 0) {
            lookup_table[n][sample][subband] |= (1<<(fan_out-1-track_nr));
          }
        }
      }
    }
  }

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {
    assert((int)output_data.size() == n_subbands);
    char *output_[output_data.size()];
    for (size_t i=0; i<output_data.size(); i++) {
      output_[i] = output_data[i];
    }
    const uint8_t *in_array = (uint8_t *)&in_data[0];

    for (int subband=0; subband<n_subbands; subband++) {
      memset((void *)output_data[subband], 0, n_input_samples*fan_out/8+1);
    }
    for (int sample=0; sample<n_input_samples+1; sample+=samples_per_byte) {
      // samples_per_byte-1 times:
      for (int i=1; i<samples_per_byte; i++) {
        process_sample(in_array, output_data);

        for (int subband=0; subband<n_subbands; subband++)
          output_data[subband][0] = (output_data[subband][0] << fan_out);
      }

      process_sample(in_array, output_data);
      for (int subband=0; subband<n_subbands; subband++)
        output_data[subband]++;
    }

    { // Do the offset
      uint8_t *mask_right = bit_shift_right_table[offset];
      for (int subband=0; subband<n_subbands; subband++) {
        // shift the last sample
        if (fan_out < 4)
          output_data[subband][0] = (output_data[subband][0]<<(8-2*fan_out));

        char *data = output_[subband];
        for (int sample=0; sample<n_input_samples*fan_out/8; sample++) {
          *data = (*data << offset) | mask_right[(uint8_t)*(data+1)];
          data++;
        }
      }
    }
  }

private:
  inline void process_sample(const uint8_t *&in_array,
                             std::vector<char *> &output_data) {
    for (int n=0; n<N; n++) {
      uint8_t *in_sample = lookup_table[n][*in_array];

      for (int subband=0; subband<n_subbands; subband++) {
        *output_data[subband] |= *in_sample;
        in_sample++;
      }

      in_array++;
    }
  }
  // Lookup table for the channel extraction
  // At most 8 channels
  // lookup_table[index in Type word][value of the byte][output sample per channel]
  uint8_t lookup_table[N][256][8];

  // Lookup table for a right bitshift with 0 inserted
  uint8_t bit_shift_right_table[8][256];

  // Fan out
  int fan_out, samples_per_byte;

  int n_subbands;
};

class Extractor4 {
public:
  Extractor4(const std::vector< std::vector<int> > &track_positions) {
    // bit shift right with padding
    for (int shift=0; shift<8; shift++) {
      for (int value=0; value<256; value++) {
        bit_shift_right_table[shift][value] =
          (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
      }
    }

    n_subbands = track_positions.size();
    fan_out = track_positions[0].size();
    samples_per_byte = 8/fan_out;
    assert(fan_out <= 8);
    assert(8%fan_out == 0);
    assert(fan_out*samples_per_byte == 8);

    memset(lookup_table, 0, N*256*8);
    // Lookup table for the tracks
    for (int subband=0; subband < n_subbands; subband++) {
      for (int track_nr=0; track_nr < fan_out; track_nr++) {
        int track = track_positions[subband][track_nr];
        int n = track/8;
        assert(n < N);
        int bit = track % 8;
        for (int sample=0; sample<256; sample++) {
          if (((sample >> bit)& 1) != 0) {
            lookup_table[n][sample][subband] |= (1<<(fan_out-1-track_nr));
          }
        }
      }
    }
  }

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {
    for (int subband=0; subband<n_subbands; subband++) {
      const uint8_t *in_pos = (const uint8_t *)in_data;
      char *out_pos = output_data[subband];

      memset((void *)out_pos, 0, n_input_samples*fan_out/8+1);

      for (int sample=0; sample<n_input_samples+1; sample+=samples_per_byte) {
        // samples_per_byte-1 times:
        for (int i=1; i<samples_per_byte; i++) {
          process_sample(in_pos, out_pos, subband);

          *out_pos = (*out_pos << fan_out);
        }

        process_sample(in_pos, out_pos, subband);
        out_pos++;
      }
      { // Do the offset
        uint8_t *mask_right = bit_shift_right_table[offset];

        // shift the last sample
        if (fan_out < 4)
          *out_pos = (*out_pos<<(8-2*fan_out));

        char *data = output_data[subband];
        for (int sample=0; sample<n_input_samples*fan_out/8; sample++) {
          *data = (*data << offset) | mask_right[(uint8_t)*(data+1)];
          data++;
        }
      }
    }

  }

private:
  inline void process_sample(const uint8_t *&in_array,
                             char *output_data,
                             int subband) {
    for (int n=0; n<N; n++) {
      *output_data |= lookup_table[n][*in_array][subband];

      in_array++;
    }
  }
  // Lookup table for the channel extraction
  // At most 8 channels
  // lookup_table[index in Type word][value of the byte][output sample per channel]
  uint8_t lookup_table[N][256][8];

  // Lookup table for a right bitshift with 0 inserted
  uint8_t bit_shift_right_table[8][256];

  // Fan out
  int fan_out, samples_per_byte;

  int n_subbands;
};

class Extractor5{
public:
  Extractor5(const std::vector< std::vector<int> > &track_positions) {
    // bit shift right with padding
    for (int shift=0; shift<8; shift++) {
      for (int value=0; value<256; value++) {
        bit_shift_right_table[shift][value] =
          (uint8_t)((value >> (8-shift)) & ((1<<shift)-1));
      }
    }

    n_subbands = track_positions.size();
    fan_out = track_positions[0].size();
    samples_per_byte = 8/fan_out;
    assert(fan_out <= 8);
    assert(8%fan_out == 0);
    assert(fan_out*samples_per_byte == 8);

    memset(lookup_table, 0, N*256*8);
    // Lookup table for the tracks
    for (int subband=0; subband < n_subbands; subband++) {
      for (int track_nr=0; track_nr < fan_out; track_nr++) {
        int track = track_positions[subband][track_nr];
        int n = track/8;
        assert(n < N);
        int bit = track % 8;
        for (int sample=0; sample<256; sample++) {
          if (((sample >> bit)& 1) != 0) {
            lookup_table[subband][n][sample] |= (1<<(fan_out-1-track_nr));
          }
        }
      }
    }
  }

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {

    for (int subband=0; subband<n_subbands; subband++) {
      const uint8_t *in_pos = (const uint8_t *)in_data;
      char *out_pos = output_data[subband];
      const uint8_t *table = &lookup_table[subband][0][0];

      memset((void *)out_pos, 0, n_input_samples*fan_out/8+1);

      for (int sample=0; sample<n_input_samples+1; sample+=samples_per_byte) {
        // samples_per_byte-1 times:
        for (int i=1; i<samples_per_byte; i++) {
          process_sample(in_pos, out_pos, table);

          *out_pos = (*out_pos << fan_out);
        }

        process_sample(in_pos, out_pos, table);
        out_pos++;
      }
      if( offset != 0 ){ // Do the offset
	 uint8_t *mask_right = bit_shift_right_table[offset];

        	// shift the last sample
       	 if (fan_out < 4)
        	  *out_pos = (*out_pos<<(8-2*fan_out));

        char *data = output_data[subband];
        for (int sample=0; sample<n_input_samples*fan_out/8; sample++) {
          *data = (*data << offset) | mask_right[(uint8_t)*(data+1)];
          data++;
        }
      }
    }

  }

private:
  inline void process_sample(const uint8_t *&in_array,
                             char *output_data,
                             const uint8_t *table) {
    for (int n=0; n<N; n++) {
      *output_data |= table[*in_array];
      table += 256;

      in_array++;
    }
  }
  // Lookup table for the channel extraction
  // At most 8 channels
  // lookup_table[index in Type word][value of the byte][output sample per channel]
  uint8_t lookup_table[8][N][256];

  // Lookup table for a right bitshift with 0 inserted
  uint8_t bit_shift_right_table[8][256];

  // Fan out
  int fan_out, samples_per_byte;

  int n_subbands;
};

class Action
{
	public:
		Action(int c, int v){
			channel = c;
			value = v;
			shift = 0;
		}
		Action(const Action& a){
			channel = a.channel;
			value = a.value;
			shift = a.shift;		
		}
		int channel;
		int value;
		int shift;
};

void find_add( std::vector<Action> *v, int channel, int value, int shift){
	for(unsigned int i=0;i<v->size();i++)
	{
		if( (*v)[i].channel == channel ){
			//std::cout << "Replacing: " <<  (*v)[i].value << " and => " << value << std::endl; 
			(*v)[i].value = (*v)[i].value | (value);	
			(*v)[i].shift = shift;			
			return;
		}
	}
	Action a(channel,value);
	v->push_back(a);
	return;
}


void find_add( std::vector<Action>& v, int channel, int value, int shift){
	for(unsigned int i=0;i<v.size();i++)
	{
		if( (v)[i].channel == channel ){
			//std::cout << "Replacing: " <<  (*v)[i].value << " and => " << value << std::endl; 
			(v)[i].value = (v)[i].value | (value);	
			(v)[i].shift = shift;			
			return;
		}
	}
	Action a(channel,value);
	v.push_back(a);
	return;
}



template<int Tn_subbands, int Tfan_out,  int Tsamples_per_byte, int Tseqsize>
class Extractor7 : public Extractor{
public:
  //int n_subbands;
  //int fan_out;
  //int samples_per_byte;
  //int seqsize = samples_per_byte*N;

  Extractor7(const std::vector< std::vector<int> > &track_positions) {
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
	for(unsigned int i=0;i<track_positions.size();i++)
	{
		std::cout << "Channel "<<i<<": ";
		for(unsigned int j=0;j<track_positions[i].size();j++)
		{
			std::cout << track_positions[i][j] << ", ";
		}
		std::cout << std::endl;
	}

	int val = Tsamples_per_byte;
	int padd = N-val;
	std::cout << "VAL: " << val << " and padding: << " << padd <<std::endl;	

	bg_size2 = Tsamples_per_byte+padd;
	bg_size1 = 256*(bg_size2);
	unsigned int big_size = Tseqsize*bg_size1;
	bigarray = new uint8_t[big_size];
	
	// Size of a the last item
	bg2_size2 = Tsamples_per_byte+padd;
	
	// sequence size
	bg2_size1 = Tseqsize*bg2_size2;
	bg2_size  = 256 * bg_size1;
	bigarray2 = new uint8_t[bg2_size];

	for(int i =0;i<256;i++){
	  iptable[i] = i*bg_size2;
	  iptable2[i] = i*bg2_size2;
	}
	
	table = new std::vector<Action>**[Tseqsize];
	//ftable = new uint8_t**[seqsize];
	for(unsigned int i=0;i<Tseqsize;i++){
		table[i] = new std::vector<Action>*[256];
		//ftable[i] = new uint8_t*[256];
		for(unsigned int j=0;j<256;j++){
			table[i][j] = new std::vector<Action>();
			//for(unsigned int k=0;k<8;k++) table[i][j][k] = 0;
		}
	}
	//ftable = new uint8_t[seqsize][256][32];
	
	int seqpref = 0;
	int cshift[256][Tn_subbands];
	for(unsigned int i=0;i<256;i++)
		for(int j=0;j<Tn_subbands;j++) 
			cshift[i][j] = 7;

	for(unsigned int r=0;r<Tseqsize;r+=Tfan_out){
	for(unsigned int i=0;i<Tn_subbands;i++)
	{		
		//std::cout << "Doing TABLE for channel "<<i<<": ";
		for(unsigned int j=0;j<track_positions[i].size();j++)
		{
			uint8_t idx = track_positions[i][j];
			//std::cout << track_positions[i][j] << ": ";
			//assert(seqidx < fan_out);			
			
			uint8_t seqidx = r  + idx / 8;
			uint8_t cidx = idx % 8;
			//std::cout << "SHIFING("<< (int)r << ":"<< (int)seqidx <<","<< (int)cidx <<"): " << std::endl;
			for(unsigned int k=0;k<256;k++){
				//if( (k>>cidx&1) )
				find_add( table[seqidx][k], i, (k>>cidx&1) << (cshift[k][i]), (cshift[k][i]) );
				(cshift[k][i])--;
			}
			//std::cout << std::endl;
			//std::cout << ", " ;	
		}
		//std::cout << std::endl;
	}
	}

	//order = new uint8_t[Tseqsize*Tsamples_per_byte];
	int tidx=0;
	memset( bigarray2, 101, bg2_size);		
	for(unsigned int i=0;i<256;i++){
		for(unsigned int j=0;j<Tseqsize;j++)
		{	
			std::vector<Action>::iterator bg = (*table[j][i]).begin();
			std::vector<Action>::iterator en = (*table[j][i]).end();
			
			memset( &(bigarray[j*(bg_size1)+i*(bg_size2)]) , 101, bg_size2);
			int idefix=0;					
			//std::cout << "[";
			while( bg != en ){
				assert(j*(bg_size1)+i*(bg_size2)+idefix < big_size);
				bigarray[j*(bg_size1)+i*(bg_size2)+idefix ] = (*bg).value;
				bigarray2[ i*bg2_size1+j*bg2_size2 +idefix ] = (*bg).value;
				//std::cout << "Value:" << i*bg2_size1+j*bg2_size2+idefix << std::endl;
				
				//for(unsigned int offset = 0; offset < Tfan_out ;offset++){
				newtab[i][j][idefix] = ((unsigned char)(*bg).value) ;
				//}

				
				idefix+=1;
				if(i == 255 ){
					std::cout << (*bg).channel << ":("<< (*bg).value << ") ";
					order[tidx] = (*bg).channel;
					std::cout << "Adding: " << tidx << " " << (*bg).channel+1 <<" " << std::endl;
							
					tidx++;				
				}
				bg++;	
				
			}
			//std::cout << "] ";

		}
		//std::cout << std::endl;
	}

	//tmpout = new char[Tn_subbands+1];
	for(unsigned int i=0; i<Tseqsize*Tsamples_per_byte;i++){
		fasttmp[i] = &(tmpout[ order[i] ] );
	}
	memset(tmpout, 0, Tn_subbands+1);   
  }

  void do_task(int n_input_samples,
               int offset,
               const Type in_data[],
               std::vector<char *> output_data) {
	       switch(offset){
			case 0: do_task_no_offset(n_input_samples, in_data, output_data); break;		
	       		case 1: do_task_offset<1, 7>(n_input_samples, in_data, output_data);break;	
	       		case 2: do_task_offset<2, 6>(n_input_samples, in_data, output_data);break;	
	       		case 3: do_task_offset<3, 5>(n_input_samples, in_data, output_data);break;
			default: 
				assert("Strange offset for this fan_out");
  		}
	}



	template<int Toffset, int Tshift> 
	void do_task_offset(int n_input_samples,
               const Type in_data[],
               std::vector<char *> output_data){
	
	unsigned int outindex=0;
    	unsigned char* currbuffer = (unsigned char*)in_data;
    	unsigned char* endbuffer = currbuffer+(n_input_samples+1)*N;
		
	//uint8_t* dorder = order;
        //char** cfasttmp = &(fasttmp[0]);
	//int shift = 8-offset;
	//shift = 8;
	for(unsigned int i=0;i<Tseqsize;++i){
			char* dst;
			unsigned char* channelidxval2;
		
			channelidxval2 = &(newtab[(*currbuffer)][i][0]);
			dst = fasttmp[i*Tsamples_per_byte];			

			for(unsigned int j=0;j<Tsamples_per_byte;j++){		
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
	        for (unsigned int i=0;i<Tn_subbands;i++)
	        {
			output_data[i][outindex] = tmpout[i];
	        	tmpout[i]=0;
	        }
	++outindex;

	while(currbuffer < endbuffer)
    	{
		for(unsigned int i=0;i<Tseqsize;++i){
			char* dst;
			unsigned char* channelidxval2;
		
			channelidxval2 = &(newtab[(*currbuffer)][i][0]);
			dst = fasttmp[i*Tsamples_per_byte];			

			for(unsigned int j=0;j<Tsamples_per_byte;j++){		
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
	        for (unsigned int i=0;i<Tn_subbands;i++)
	        {
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
               			const Type in_data[],
               			std::vector<char *> output_data){
	
	unsigned int outindex=0;
    	unsigned char* currbuffer = (unsigned char*)in_data;
    	unsigned char* endbuffer = currbuffer+(n_input_samples)*N;
	
	while(currbuffer < endbuffer)
    	{
		for(unsigned int i=0;i<Tseqsize;++i){
			char* dst;
			unsigned char* channelidxval2;
		
			channelidxval2 = &(newtab[(*currbuffer)][i][0]);
			dst = fasttmp[i*Tsamples_per_byte];			

			for(unsigned int j=0;j<Tsamples_per_byte;j++){		
				*(dst+j) |= (*(channelidxval2+j));
			}
			++currbuffer;
		}
	        for (unsigned int i=0;i<Tn_subbands;i++)
	        {
			output_data[i][outindex] = tmpout[i];
	        	tmpout[i]=0;
	        }		
		++outindex;
	    }	
	}
private:
  std::vector<Action> ***table;
  uint8_t ***ftable;

  uint8_t *bigarray;
  unsigned int bg_size1;
  unsigned int bg_size2;
		
  unsigned int iptable[256];
  unsigned int iptable2[256];

  unsigned int bg2_size1;
  unsigned int bg2_size2;
  unsigned int bg2_size;
  uint8_t* bigarray2;

  uint8_t  order[Tseqsize*Tsamples_per_byte];
  char* fasttmp[Tseqsize*Tsamples_per_byte];

  uint8_t  newtab[256][Tseqsize][Tsamples_per_byte];	
  
  char tmpout[Tn_subbands+1];
};


bool check_buffers(std::vector<char *> &output_buffers,
                   std::vector<char *> &output_buffers2,
                   int buffer_size) {
  assert(output_buffers.size() == output_buffers2.size());
  assert(buffer_size > 0);
  for (size_t i=0; i<output_buffers.size(); i++) {
    for (int j=0; j<buffer_size; j++) {
      if (output_buffers[i][j] != output_buffers2[i][j]) {
        std::cout << "mismatch at subband " << i << " byte " << j << std::endl;
        return false;
      }
    }
  }
  return true;
}



void print(std::vector<char *> &output_buffers,
           int buffer_size) {
#ifdef VERBOSE
  assert(buffer_size > 0);
  //  for (size_t j=0; j<output_buffers.size(); j++)
  //    if (j<10)
  //      std::cout << "0" << j << " ";
  //    else
  //      std::cout << j << " ";
  //  std::cout << std::endl;
  //  for (size_t j=0; j<output_buffers.size(); j++)
  //    std::cout << "---";
  //  std::cout << std::endl;
  for (int j=0; j<buffer_size; j++) {
    for (size_t i=0; i<output_buffers.size(); i++) {
      print_hex(std::cout, output_buffers[i][j]);
      std::cout << " ";
    }
    std::cout << std::endl;
  }
#endif // VERBOSE

}

//void randomize_buffers(std::vector<char *> &output_buffers,
//                       int buffer_size) {
//  for (size_t i=0; i<output_buffers.size(); i++) {
//    for (int j=0; j<buffer_size; j++) {
//      output_buffers[i][j] = random();
//    }
//  }
//}


//#include "extractor7.hh"
#include "extractor8.inl.cc"
class ExtractorFactory
{	
	public:
		static Extractor* get_new_extractor(const std::vector< std::vector<int> > &track_positions){
			switch( track_positions.size() ){
				case 4:
					// 1) n_subbands
					// 2) fan_out = (8*N)/n_subbands;
					// 3) samples_per_byte = 8/fan_out;
					// 4) seqsize = samples_per_byte*N;
					return new Extractor7<4, 8*TN/4, 1/(TN/4), TN/(TN/4) >(track_positions);
				case 8:
//					return get_extractor8(  track_positions );
					return new Extractor7<8, (8*TN)/8, 8/((8*TN)/8), (8/((8*TN)/8))*N >(track_positions);	
				default:	
					assert( 0 && "This extractor is not supported" );
					std::cout<< "NO CHANNELIZER SUPPORTED" << std::endl;					
					exit(0);		
			}	
		} 	


};

void test(const int n_subbands,
          const int fan_out,
          const int in_size,
          const int offset,
          const int n_iterations,
          const Type in_data[],
          const std::vector< std::vector<int> > &tracks_in_subbands,
          bool show_timings = true) {

  // Output buffers
  std::vector<char *> output_buffers, output_buffers2;
  output_buffers.resize(n_subbands);
  output_buffers2.resize(n_subbands);
  for (int i=0; i<n_subbands; i++) {
    output_buffers[i] = new char[in_size*fan_out/8+1];
    output_buffers2[i] = new char[in_size*fan_out/8+1];
  }

  { // Fill reference data
    Extractor1 extractor1(tracks_in_subbands);
    extractor1.do_task(in_size, offset, in_data, output_buffers);


  }

  if (false) {
    Extractor1 extractor1(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);

if (show_timings)    std::cout << "Extractor1: starting bench"  << std::endl;

    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor1.do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor1: " << timer.measured_time() << std::endl;

    //    print(output_buffers2, in_size*fan_out/8);

    // Check data
    if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8))
      std::cout << "Extractor1: Output data does not match" << std::endl;

  }

  if (false) {
    Extractor2 extractor2(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);
 
if (show_timings)    std::cout << "Extractor2: starting bench"  << std::endl;

    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor2.do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor2: " << timer.measured_time() << std::endl;

    //    print(output_buffers2, in_size*fan_out/8);

    // Check data
    if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8)) {
      std::cout << "Extractor2: Output data does not match" << std::endl;
      assert(false);
    }

  }

  if (false) {
    Extractor3 extractor3(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);
if (show_timings)   std::cout << "Extractor3: starting bench"  << std::endl;

    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor3.do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor3: " << timer.measured_time() << std::endl;

    //    print(output_buffers2, in_size*fan_out/8);

    // Check data
    if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8)) {
      std::cout << "Extractor3: Output data does not match" << std::endl;
      for (int i=0; i<in_size; i++)
        print_hex(std::cout, in_data[i]) << std::endl;
      std::cout << " ---" << std::endl;
      print(output_buffers, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      print(output_buffers2, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      assert(false);
    }

  }

  if (false) {
    Extractor4 extractor4(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);
    if (show_timings) std::cout << "Extractor4: starting bench"  << std::endl;

    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor4.do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor4: " << timer.measured_time() << std::endl;

    //    print(output_buffers2, in_size*fan_out/8);

    // Check data
    if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8)) {
      std::cout << "Extractor4: Output data does not match" << std::endl;
      for (int i=0; i<in_size; i++)
        print_hex(std::cout, in_data[i]) << std::endl;
      std::cout << " ---" << std::endl;
      print(output_buffers, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      print(output_buffers2, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      assert(false);
    }

  }

  if (true) {
    Extractor5 extractor5(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);

    if (show_timings)	
    	std::cout << "Extractor5: starting bench: "  << in_size << std::endl;


    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor5.do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor5: " << timer.measured_time() << std::endl;

    //    print(output_buffers2, in_size*fan_out/8);

    // Check data
    if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8)) {
      std::cout << "Extractor5: Output data does not match" << std::endl;
      for (int i=0; i<in_size; i++)
        print_hex(std::cout, in_data[i]) << std::endl;
      std::cout << " ---" << std::endl;
      print(output_buffers, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      print(output_buffers2, in_size*fan_out/8);
      std::cout << " ---" << std::endl;
      assert(false);
    }
  }

if (true) {
    Extractor* extractor7 = ExtractorFactory::get_new_extractor(tracks_in_subbands);
    // reset output buffers
    randomize_buffers(output_buffers2, in_size*fan_out/8);

    if (show_timings)	
    	std::cout << "Extractor7: starting bench: "  << in_size << std::endl;

    Timer timer;
    timer.resume();
    for (int i=0; i<n_iterations; i++) {
      extractor7->do_task(in_size, offset, in_data, output_buffers2);
    }
    timer.stop();
    if (show_timings)
      std::cout << "Extractor7: " << timer.measured_time() << std::endl;

    // Check data
	
   if (!check_buffers(output_buffers, output_buffers2, in_size*fan_out/8)) {
      std::cout << "Extractor7: Output data does not match" << std::endl;
      for (int i=0; i<32; i++)
        print_hex(std::cout, in_data[i]) << std::endl;
      	std::cout << " ---" << std::endl;
      	print(output_buffers, in_size*fan_out/8);
      	std::cout << " ---" << std::endl;
      	print(output_buffers2, in_size*fan_out/8);
      	std::cout << " ---" << std::endl;
      assert(false);
    }
  }

  

  for (int i=0; i<n_subbands; i++) {
    delete[] output_buffers[i];
    delete[] output_buffers2[i];
  }
}


int main() {
  int n_iterations = 1;

  int n_subbands = 8;
  int fan_out    = (8*N)/n_subbands;
  int in_size    = 1048576;
  //int in_size = 32;
  int offset     = 0;

  assert(n_subbands * fan_out <= in_size);
  assert(offset < fan_out);

  // Input buffer
  Type in_data[in_size+1];
  for (int i=0; i<=in_size; i++)
    in_data[i] = (i<in_size/2 ? i : -i);
  for (int i=0; i<=in_size; i++)
    in_data[i] = rand();
  for (int i=0; i<=in_size; i++)
    in_data[i] = i;
  for (int i=0; i<=in_size; i++)
    in_data[i] = (1<<(i%(8*N)));
  for (int i=0; i<=in_size; i++)
    in_data[i] = (0xaaaaaaaa) + i;


  if (false) { // thorough check
    for (int n_subbands=1; n_subbands<8; n_subbands++) {

      int fan_out=1;
      while ((fan_out <= 8) && (n_subbands * fan_out < N*8)) {
        // Tracks
        std::vector< std::vector<int> > tracks_in_subbands;
        tracks_in_subbands.resize(n_subbands);
        for (int i=0; i<n_subbands; i++)
          tracks_in_subbands[i].resize(fan_out);
        assert(n_subbands*fan_out <= N*8);
        for (int i=0; i<n_subbands*fan_out; i++)
          tracks_in_subbands[i%n_subbands][i/n_subbands] = i;
        for (int i=0; i<n_subbands*fan_out; i++)
          tracks_in_subbands[i/fan_out][i%fan_out] = rand()%(8*N);
        for (int i=0; i<n_subbands*fan_out; i++)
          tracks_in_subbands[i/fan_out][i%fan_out] = i;

        for (int offset = 0; offset < fan_out; offset++) {
#ifdef VERBOSE
          std::cout << "(subband, fan_out, offset) = "
          << "(" << n_subbands << ", " << fan_out << ", " << offset << ")" << std::endl;
#if 0

          for (int subband=0; subband<n_subbands; subband++) {
            std::cout << "Subband: " << subband <<": ";
            for (int i=0; i<fan_out; i++) {
              std::cout << tracks_in_subbands[subband][i] << " ";
            }
            std::cout << std::endl;
          }
#endif
#endif // VERBOSE

          test(n_subbands, fan_out, in_size, offset, n_iterations,
               in_data, tracks_in_subbands, false);
        }
        fan_out *= 2;
      }
    }
  }


  // Tracks
  std::vector< std::vector<int> > tracks_in_subbands;
  tracks_in_subbands.resize(n_subbands);
  for (int i=0; i<n_subbands; i++)
    tracks_in_subbands[i].resize(fan_out);
  assert(n_subbands*fan_out <= N*8);
  for (int i=0; i<n_subbands*fan_out; i++)
    tracks_in_subbands[i%n_subbands][i/n_subbands] = i;
  for (int i=0; i<n_subbands*fan_out; i++)
    tracks_in_subbands[i/fan_out][i%fan_out] = i;

#if 0

  for (int subband=0; subband<n_subbands; subband++) {
    std::cout << "Subband: " << subband <<": ";
    for (int i=0; i<fan_out; i++) {
      std::cout << tracks_in_subbands[subband][i] << " ";
    }
    std::cout << std::endl;
  }
#endif

  n_iterations = 300;
  std::cout << "Processing: " << (((uint64_t)n_iterations*in_size*N))/(1024*1024) << "MB" << std::endl;
  test(n_subbands, fan_out, in_size, offset, n_iterations,
       in_data, tracks_in_subbands);
}
