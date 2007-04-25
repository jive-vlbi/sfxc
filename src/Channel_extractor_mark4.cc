/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Buffer.h 191 2007-04-05 11:34:41Z kruithof $
 *
 */

#include "Channel_extractor_mark4.h"
#include <genFunctions.h>
#include "Mark4_header.h"

#include <assert.h>


// Templated by the type of the element from which the samples are extracted
// Either INT32 (n_head_stacks == 1) or INT64 (n_head_stacks == 2)
template <class T>
class Channel_extractor_mark4_implementation {
public:
  typedef Channel_extractor_mark4::DEBUG_LEVEL  DEBUG_LEVEL;

  Channel_extractor_mark4_implementation(Data_reader &reader, StaP &staPrms,
                                         DEBUG_LEVEL debug_level);

  void goto_time_stamp(INT64 time);
  INT64 get_last_time_stamp();

  UINT64 get_bytes(UINT64 nBytes, char *buff);
  
  bool eof();

  bool check_track_bit_statistics();  
  
private:

  // Finds the header on the track of the first sign bit
  int find_header();

  bool increase_current_position_in_block();

  int read_new_block();
  
  void check_time_stamp();
  
  Data_reader &reader;
  
  /// The number of samples per 32/64 bit integer
  int fan_out;

  /// Number of bits per sample (sign or sign+magn)
  int n_bits_per_sample;
  
  /// Bit positions for the sign and magnitude bits 
  std::vector<int> tracks;

  /// Insertion of random bits for the headers, to remove a false signal
  bool insert_random_headers; 

  T block[frameMk4];
  int curr_pos_in_block;
  
  Mark4_header<T> mark4_header;
  
  INT32 start_day;
  INT64 start_microtime;
  INT32 TBR;
  DEBUG_LEVEL debug_level;
  int block_count;
};


Channel_extractor_mark4::
Channel_extractor_mark4(Data_reader &reader, 
                        StaP &staPrms, 
                        DEBUG_LEVEL debug_level)
 : Data_reader(),
   n_head_stacks(staPrms.get_nhs()),
   ch_extractor_1_head_stack(NULL),
   ch_extractor_2_head_stack(NULL)
{
  if (n_head_stacks == 1) {
    ch_extractor_1_head_stack = 
      new Channel_extractor_mark4_implementation<UINT32>(reader, staPrms, debug_level);
  } else {
    assert (n_head_stacks == 2);
    ch_extractor_2_head_stack = 
      new Channel_extractor_mark4_implementation<UINT64>(reader, staPrms, debug_level);
  }
}

void 
Channel_extractor_mark4::goto_time_stamp(INT64 time) {
  if (n_head_stacks == 1) {
    ch_extractor_1_head_stack->goto_time_stamp(time);
  } else {
    ch_extractor_2_head_stack->goto_time_stamp(time);
  }
}
INT64 
Channel_extractor_mark4::get_last_time_stamp() {
  if (n_head_stacks == 1) {
    return ch_extractor_1_head_stack->get_last_time_stamp();
  } else {
    return ch_extractor_2_head_stack->get_last_time_stamp();
  }
}

size_t 
Channel_extractor_mark4::do_get_bytes(size_t nBytes, char *buff) {
  if (n_head_stacks == 1) {
    return ch_extractor_1_head_stack->get_bytes(nBytes, buff);
  } else {
    return ch_extractor_2_head_stack->get_bytes(nBytes, buff);
  }
}

bool Channel_extractor_mark4::eof() {
  if (n_head_stacks == 1) {
    return ch_extractor_1_head_stack->eof();
  } else {
    return ch_extractor_2_head_stack->eof();
  }
}
  
/*********************************************************************
 * Implementation of the Channel_extractor for Mark 4 files (32 or 64 bit)
 *********************************************************************/

template <class T>
Channel_extractor_mark4_implementation<T>::
Channel_extractor_mark4_implementation(Data_reader &reader, 
                                       StaP &staPrms, 
                                       DEBUG_LEVEL debug_level)
 : reader(reader),
   fan_out(staPrms.get_fo()), 
   n_bits_per_sample(staPrms.get_bps()),
   insert_random_headers(staPrms.get_rndhdr()),
   curr_pos_in_block(0),
   TBR(staPrms.get_tbr()),
   debug_level(debug_level),
   block_count(0)
{
  
  assert(staPrms.get_tphs()*staPrms.get_nhs() == sizeof(T)*8);
  
  // Make sure the header starts on the first byte:
  reader.get_bytes(frameMk4*sizeof(T), (char *)block);
  
  int header_start = find_header();
  if (header_start < 0) {
    // Header not found. It probably lies on the boundary of the processed block
    // Read a half block and retry
    int mid = (frameMk4*sizeof(T))/2+1;
    int end = frameMk4*sizeof(T);
    memcpy(block, block + mid, end-mid);
    reader.get_bytes((UINT64)(end - mid), (char*)(block + mid));
    header_start = find_header();
  }
  assert(header_start >= 0);
  
  memmove(block, block + header_start, header_start*sizeof(T));
  reader.get_bytes(header_start*sizeof(T), (char *)&block[frameMk4-header_start]);

  assert(find_header() == 0);
  mark4_header.set_header(block);
  mark4_header.check_header();
  
  start_day = mark4_header.day(0);
  start_microtime = mark4_header.get_microtime(0);
  reader.reset_data_counter();

  // Store a list of tracks: First sign then (optionally) magnitude
  tracks.resize(n_bits_per_sample*fan_out);
  for (int i=0; i<fan_out; i++) {
    if (! mark4_header.is_sign(staPrms.get_signBS()[i])) {
      std::cout << "Track " << staPrms.get_signBS()[i]
                << " is not a sign track" << std::endl;
    }
    tracks[n_bits_per_sample*i] = staPrms.get_signBS()[i];
    if (n_bits_per_sample > 1) {
      if (! mark4_header.is_magn(staPrms.get_magnBS()[i])) {
        std::cout << "Track " << staPrms.get_magnBS()[i]
                  << " is not a magn track" << std::endl;
      }
      tracks[n_bits_per_sample*i+1] = staPrms.get_magnBS()[i];
    }
  }
  std::cout << std::endl;
}

template <class T>
void
Channel_extractor_mark4_implementation<T>::
goto_time_stamp(INT64 time) {
  assert(false);
}

template <class T>
INT64
Channel_extractor_mark4_implementation<T>::
get_last_time_stamp() {
  return mark4_header.get_microtime(tracks[0]);
}

template <class T>
UINT64
Channel_extractor_mark4_implementation<T>::
get_bytes(UINT64 nOutputBytes, char *output_buffer) {
  UINT64 bytes_processed = 0;
  
  // Initialise the output buffer:
  memset(output_buffer, 0, nOutputBytes);
  
  while (bytes_processed < nOutputBytes) {
    // Fill the output buffer
    // Filled from least to most significant bit

    // Position in the output byte:
    int sample;
    for (int sample_pos=0; sample_pos<8;) {
      for (typename std::vector<int>::iterator it = tracks.begin();
           it != tracks.end(); it++) {
        //get sign and magnitude bit for all channels
        if (insert_random_headers && (curr_pos_in_block < 160)) {
          sample = irbit2();
        } else {
          sample = ( block[curr_pos_in_block]>>(*it) ) & 1;
        }
        // insert the sample into the output buffer
        output_buffer[bytes_processed] |= (sample << sample_pos);
        // Shift two positions in the output buffer
        sample_pos ++;
      }
      if (!increase_current_position_in_block()) {
        // End of data
        return bytes_processed;
      }
    }
    bytes_processed++;
  }
              
  return bytes_processed;
}
  
template <class T>
bool
Channel_extractor_mark4_implementation<T>::
increase_current_position_in_block() {
  curr_pos_in_block++;
  if (curr_pos_in_block == frameMk4) {
    int result = read_new_block();
    if (result != frameMk4) {
      // Could not read a new block: terminating
      curr_pos_in_block = frameMk4-1;
      return false;
    }
    curr_pos_in_block = 0;
  }
  assert(curr_pos_in_block < frameMk4);
  return true;
}
  
template <class T>
int
Channel_extractor_mark4_implementation<T>::
read_new_block() {
  int result = reader.get_bytes(frameMk4*sizeof(T),(char *)block)/sizeof(T);
  if (result != frameMk4*sizeof(T)) return result;

  if (debug_level >= Channel_extractor_mark4::CHECK_PERIODIC_HEADERS) {
    if ((debug_level >= Channel_extractor_mark4::CHECK_ALL_HEADERS) ||
        ((++block_count % 100) == 0)) {
      assert(find_header() == 0);
      mark4_header.check_header();
      check_time_stamp();
      if (Channel_extractor_mark4::CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics()) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }

  return result;
}

template <class T>
void
Channel_extractor_mark4_implementation<T>::
check_time_stamp() {
  double delta_time = 
    mark4_header.get_microtime_difference(start_day, start_microtime, tracks[0])/1000000.;
  
  double computed_TBR = (reader.data_counter()*8/1000000.) / 
                        (delta_time * sizeof(T) * 8);
  
  assert (computed_TBR == TBR);
//  static int count=0;
//  if (count++ == 500) {
//    count = 0;
//    std::cout << "TBR: " 
//              << (reader.data_counter()*8/1000000.)/(delta_time * sizeof(T) * 8) 
//              << std::endl;
//  }

//  assert(false);
}

template <class T>
bool
Channel_extractor_mark4_implementation<T>::
eof() {
  return reader.eof();
}

template <class T>
int
Channel_extractor_mark4_implementation<T>::
find_header() {
  int track = 0;
  
  int nOnes = 0;
  UINT32 start_header;
  for (start_header=0; 
       (start_header<(frameMk4*sizeof(T)-32)) && (nOnes <32); 
       start_header++) {
    if ((block[start_header] >> track) & 1) {
      nOnes ++;
    } else {
      nOnes = 0;
    } 
  }
  // Check whether we found an entire header:
  if ((nOnes == 32) && (start_header >= 96)) {
    return start_header-96;
  } 
  return -1;
}

template <class T>
bool
Channel_extractor_mark4_implementation<T>::
check_track_bit_statistics() {
  double track_bit_statistics[sizeof(T)*8];
  for (size_t track=0; track<sizeof(T)*8; track++) {
    track_bit_statistics[track]=0;
  }
  
  for (int i=160; i<frameMk4; i++) {
    for (size_t track=0; track<sizeof(T)*8; track++) {
      track_bit_statistics[track] += (block[i] >> track) &1;
    }
  }
  
  for (size_t track=0; track<sizeof(T)*8; track++) {
    track_bit_statistics[track] /= frameMk4;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }    
  
  return true;
}
