/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef MARK4_HEADER_H
#define MARK4_HEADER_H

#include <assert.h>
#include <utils.h>

/** Mark IV frame header in byte chunks, without parity bits. */
template <class T>
class Mark4_header  {
public:
  Mark4_header();
  
  void set_header(T *header);
  bool check_header();
  
  int nTracks();
  
  int headstack(int track);
  int track(int track);
  bool is_sign(int track);
  bool is_magn(int track);
  
  int year(int track);
  int day(int track);
  int hour(int track);
  int minute(int track);
  int second(int track);
  int milisecond(int track);
  // Track is not used, just to make the function idiot proof ( me :) )
  int microsecond(int track, int milisecond);

  int find_track(int headstack, int track);
  
  int get_time_in_ms(int track);
  std::string get_time_str(int track);

  bool checkCRC();
  void recomputeCRC();

  bool is_valid();
  
  static const int microsecond_offset[];

  Log_writer &print_binary_header(Log_writer &writer);
private:  
  void crc12(T *crcBlock, T *data, int datawords);
  
  // Binary coded decimal:
  int BCD(T *start, int track);

private:
  T* header;

  static const char *header_map[];
};

template <class T>
const int Mark4_header<T>::microsecond_offset[] = 
  {0, 250, 500, 750, /*invalid*/-1, 0, 250, 500, 750, /*invalid*/-1};


template <class T>
Mark4_header<T>::Mark4_header() : header(NULL) {
}

template <class T>
void Mark4_header<T>::set_header(T* hdr) {
  header = hdr;
}

template <class T>
bool Mark4_header<T>::check_header() {
  // Check for a valid header
  assert(header != NULL);
  if (!is_valid()) return false;
  if (!checkCRC()) return false;
  return true;
}


template <class T>
bool Mark4_header<T>::is_valid() {
  for (int i=64; i<96; i++) {
    assert(T(-1)^T(0) == T(-1));
    if (header[i] != T(-1)) {
      char word[8*sizeof(T)];
      for (int j=64; j<96; j++) {
        itoa(header[j], word, 2);
        printf(" Word: %03d %16s\n", j, word);
      }
      std::cout << " No synchword found " << i << std::endl;
      return false;
    }
  }
  return true;
}

template <class T>
int Mark4_header<T>::nTracks() {
  return 8*sizeof(T);
}

template <class T>
int Mark4_header<T>::year(int track) {
  assert((track >= 0) && (track < (int)sizeof(T)*8));
  return 2000 + BCD(header + 96+ 0, track);
}

template <class T>
int Mark4_header<T>::day(int track) {
  return BCD(header + 96+ 4, track)*100 +
         BCD(header + 96+ 8, track)*10 +
         BCD(header + 96+12, track);
}

template <class T>
int Mark4_header<T>::hour(int track) {
  return BCD(header + 96+16, track)*10 + BCD(header + 96+20, track);
}

template <class T>
int Mark4_header<T>::minute(int track) {
  return BCD(header + 96+24, track)*10 + BCD(header + 96+28, track);
}

template <class T>
int Mark4_header<T>::second(int track) {
  return BCD(header + 96+32, track)*10 + BCD(header + 96+36, track);
}

template <class T>
int Mark4_header<T>::milisecond(int track) {
  int unit = BCD(header + 96+48, track);
  assert(unit != 4);
  assert(unit != 9);
  return BCD(header + 96+40, track)*100 + BCD(header + 96+44, track)*10 + unit;
}
template <class T>
int Mark4_header<T>::microsecond(int track, int milisec) {
  int unit = milisec%10;
  assert(unit != 4);
  assert(unit != 9);
  return microsecond_offset[unit];
}

template <class T>
int Mark4_header<T>::headstack(int track) {
  return 2*(((*(header+32)) >> track)&1) + (((*(header+33)) >> track)&1);
}
template <class T>
int Mark4_header<T>::track(int track) {
  return 
    10*(2*(((*(header+34))>>track)&1) + (((*(header+35))>>track)&1)) +
    BCD(header+36,track);
}
template <class T>
bool Mark4_header<T>::is_sign(int track) {
  return ((((*(header+41)) >> track) &1) == 0);
}
template <class T>
bool Mark4_header<T>::is_magn(int track) {
  return ((((*(header+41)) >> track) &1) == 1);
}

template <class T>
int Mark4_header<T>::find_track(int headstack_, int track_) {
  for (int i=0; i<nTracks(); i++) {
    if ((headstack(i) == headstack_) && (track(i) == track_)) {
      return i;
    }
  }
  assert(false);
  return -1;
}


template <class T>
bool Mark4_header<T>::checkCRC() {
  T crcBlock[12];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 12*sizeof(T));
    
  /* Re-calc CRC12. Buffer is 148 bits in size, CRC is 12 */
  crc12(crcBlock, header, 160);
    
  for (int i=-0; i<12; i++) {
    if (crcBlock[i] != 0) {
      std::cout << "Error in crc " << i << " "
                << std::hex << crcBlock[i] << std::dec 
                << std::endl;
      return false;
    } 
    assert(crcBlock[i] == 0);
  }
  
  return true;
}

template <class T>
void Mark4_header<T>::recomputeCRC() {
  T crcBlock[12];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 12*sizeof(T));
    
  /* Re-calc CRC12. Buffer is 148 bits in size, CRC is 12 */
  crc12(crcBlock, header, 148);
    
  for (int i=0; i<12; i++) {
    header[148+i] = crcBlock[11-i];
  }

  checkCRC();
}

template <class T>
void Mark4_header<T>::crc12(T *crcBlock, T *data, int datawords)
{
  for (int i=64; i<96; i++) {
    assert(T(-1)^T(0) == T(-1));
    assert(header[i] == T(-1));
  }
  T *d = data;

  /* 'cr' CRC register is assumed to be initialized */
  /* with 12 data words. */
  /* (Init to all zero; or use old regs to continue. */
  for (int i=0; i<datawords; i++) {
    T n0  = (*d++) ^ crcBlock[11];  /* x^12 + */
    T n11 = n0 ^ crcBlock[10];  /* x^11 + */
    T n3  = n0 ^ crcBlock[2];   /* x^3 + */
    T n2  = n0 ^ crcBlock[1];   /* x^2 + */
    T n1  = n0 ^ crcBlock[0];   /* x + 1 */

    crcBlock[11] = n11;
    crcBlock[10] = crcBlock[ 9];
    crcBlock[ 9] = crcBlock[ 8];
    crcBlock[ 8] = crcBlock[ 7];
    crcBlock[ 7] = crcBlock[ 6];
    crcBlock[ 6] = crcBlock[ 5];
    crcBlock[ 5] = crcBlock[ 4];
    crcBlock[ 4] = crcBlock[ 3];
    crcBlock[ 3] = n3;
    crcBlock[ 2] = n2;
    crcBlock[ 1] = n1;
    crcBlock[ 0] = n0;
  }  /* for every data word (with multiple bits) in input data stream */
  /* The result is in 'cr[]', MSB in last data word. */
}  /* crc12 */

template <class T>
int 
Mark4_header<T>::BCD(T *start, int track) {
  int result = ((*start>>track)&1)*8 +
               ((*(start+1)>>track)&1)*4 +
               ((*(start+2)>>track)&1)*2 +
               ((*(start+3)>>track)&1);
  if (result >= 10) {
    std::cout << "BCD >= 10" << std::endl;
  }
  return result;
}

template <class T>
int
Mark4_header<T>::get_time_in_ms(int track) {
  int result = 
    milisecond(track) + 
    1000*(second(track) + 
          60 * (minute(track) + 
                60*(int64_t)hour(track)));
//   DEBUG_MSG("time: " << result);
  return result;
} 

template <class T>
std::string
Mark4_header<T>::get_time_str(int track) {
  char time_str[40];
  snprintf(time_str,40, "%04dy%03dd%02dh%02dm%02ds%03dms%03dus",
	   year(track),
	   day(track),
	   hour(track),
	   minute(track),
	   second(track),
           milisecond(track),
	   microsecond(track, milisecond(track)));
  return std::string(time_str);
}

template <class T>
Log_writer &
Mark4_header<T>::print_binary_header(Log_writer &writer) {
  for (int i=0; i<180; i++) {
    for (size_t track=0; track<sizeof(T)*8; track++) {
      writer << (((*(header+i)) >> track) & 1);
    }
    writer << " " << i << " " << header_map[i] << std::endl;
  }

  return writer;
}


template <class T>
const char *Mark4_header<T>::header_map[] = 
  {"0hp15",
   "0hp14",
   "0hp13",
   "0hp12",
   "0hp11",
   "0hp10",
   "0hp09",
   "0hp08",
   "0hp07",
   "0hp06",
   "0hp05",
   "0hp04",
   "0hp03",
   "0hp02",
   "0hp01",
   "0hp00",
   "1hp15",
   "1hp14",
   "1hp13",
   "1hp12",
   "1hp11",
   "1hp10",
   "1hp09",
   "1hp08",
   "1hp07",
   "1hp06",
   "1hp05",
   "1hp04",
   "1hp03",
   "1hp02",
   "1hp01",
   "1hp00",
   ".hds1",
   ".hds0",
   ".trc5",
   ".trc4",
   ".trc3",
   ".trc2",
   ".trc1",
   ".trc0",
   ".fnt1",
   ".fnt0",
   "..S/M",
   "..U/L",
   ".bbc3",
   ".bbc2",
   ".bbc1",
   ".bbc0",
   "...t7",
   "...t6",
   "...t5",
   "...t4",
   "...t3",
   "...t2",
   "...t1",
   "...t0",
   "...u7",
   "...u6",
   "...u5",
   "...u4",
   "...u3",
   "...u2",
   "...u1",
   "...u0",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "synch",
   "...y3",
   "...y2",
   "...y1",
   "...y0",
   "100d3",
   "100d2",
   "100d1",
   "100d0",
   ".10d3",
   ".10d2",
   ".10d1",
   ".10d0",
   "..1d3",
   "..1d2",
   "..1d1",
   "..1d0",
   ".10h3",
   ".10h2",
   ".10h1",
   ".10h0",
   "..1h3",
   "..1h2",
   "..1h1",
   "..1h0",
   ".10m3",
   ".10m2",
   ".10m1",
   ".10m0",
   "..1m3",
   "..1m2",
   "..1m1",
   "..1m0",
   ".10s3",
   ".10s2",
   ".10s1",
   ".10s0",
   "..1s3",
   "..1s2",
   "..1s1",
   "..1s0",
   "100m3",
   "100m2",
   "100m1",
   "100m0",
   ".10m3",
   ".10m2",
   ".10m1",
   ".10m0",
   "..1m3",
   "..1m2",
   "..1m1",
   "..1m0",
   "crc11",
   "crc10",
   "crc09",
   "crc08",
   "crc07",
   "crc06",
   "crc05",
   "crc04",
   "crc03",
   "crc02",
   "crc01",
   "crc00",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat.",
   ".dat."};

#endif // MARK4_HEADER_H
