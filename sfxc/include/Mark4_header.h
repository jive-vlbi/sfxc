/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Buffer.h 191 2007-04-05 11:34:41Z kruithof $
 *
 */

#ifndef MARK4_HEADER_H
#define MARK4_HEADER_H

#include <assert.h>

/** Mark IV frame header in byte chunks, without parity bits. */
template <class T>
class Mark4_header  {
public:
  Mark4_header();
  
  void set_header(T *header);
  void check_header();
  
  int year(int track);
  int day(int track);
  int hour(int track);
  int minute(int track);
  int second(int track);
  int microsecond(int track);
  
  bool is_sign(int track);
  bool is_magn(int track);
  
  INT64 get_microtime_difference(INT32 day, INT64 utime, int track); 
  INT64 get_microtime(int track);

  bool checkCRC();
  void recomputeCRC();

  bool is_valid();
  
  static const int microsecond_offset[];
  
private:  
  void crc12(T *crcBlock, T *data, int datawords);
  
  // Binary coded decimal:
  int BCD(T *start, int track);

private:
  T* header;
};

template <class T>
const int Mark4_header<T>::microsecond_offset[] = 
    {0, 1250, 2500, 3750, -1, 5000, 6250, 7500, 8750, -1};


template <class T>
Mark4_header<T>::Mark4_header() : header(NULL) {
//  memcpy(data_aaaa, start, 160*sizeof(tD));
//  write();
//  checkCRC();
}

template <class T>
void Mark4_header<T>::set_header(T* hdr) {
  header = hdr;
}

template <class T>
void Mark4_header<T>::check_header() {
  // Check for a valid header
  assert(is_valid());
  assert(checkCRC());
}


template <class T>
bool Mark4_header<T>::is_valid() {
  for (int i=64; i<96; i++) {
    if (!(header[i] == (~T(0)))) {
      std::cout << " No synchword found" << i << std::endl;
    }
  }
  return true;
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
int Mark4_header<T>::microsecond(int track) {
  INT64 milisec = BCD(header + 96+40, track)*100000 + 
                  BCD(header + 96+44, track)*10000;
  int unit =      BCD(header + 96+48, track);
  assert(unit != 4);
  assert(unit != 9);
  return milisec + microsecond_offset[unit];
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
    assert(header[i] == (~T(0)));
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
  assert(result < 10);
  return result;
}

template <class T>
INT64
Mark4_header<T>::get_microtime(int track) {
  return microsecond(track) + 
         1000000*(second(track) + 
                  60 * (minute(track) + 
                        60*hour(track)));
} 

template <class T>
INT64 
Mark4_header<T>::get_microtime_difference(INT32 start_day, INT64 start_microtime, int track) {
  return get_microtime(track) + (day(track) - start_day)*1000*60*60*24 - start_microtime;
} 

#endif // MARK4_HEADER_H
