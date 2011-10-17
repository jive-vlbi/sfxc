#include <stdio.h>
#include <cstring>
#include "mark5a_header.h"
#include "backtrace.h"

const int Mark5a_header::microsecond_offset[] = {
      0, 250, 500, 750, /*invalid*/-1, 0, 250, 500, 750, /*invalid*/-1
    };



Mark5a_header::Mark5a_header(int N_) : header_(NULL), N(N_) {
  SFXC_ASSERT(N_ > 0);
}


void Mark5a_header::set_header(unsigned char* hdr) {
  if (hdr == NULL) 
    std::cout << "Backtrace: " << Backtrace() << std::endl;
  SFXC_ASSERT(hdr != NULL);
  header_ = hdr;
}


bool Mark5a_header::check_header(uint8_t mask) {
  // Check for a valid header
  SFXC_ASSERT(header_ != NULL);
  if (!is_valid(mask))
    return false;
  if (!checkCRC(mask))
    return false;
  return true;
}



bool Mark5a_header::is_valid(uint8_t mask) {
  for (size_t i=64*N; i<96*N; i++) {
    if ((header_[i] & mask) != (((unsigned char)(-1)) & mask)) {
//      char word[8*N];
//      for (int j=64*N; j<96*(int)N; j++) {
//       itoa(header_[j], word, 2);
//        printf(" Word: %03d %16s\n", j, word);
//      }
      std::cout << "Invalid Mark5a frame : No syncword found " << i << std::endl;
      return false;
    }
  }
  return true;
}

uint64_t
Mark5a_header::get_track_mask() {
  uint64_t mask;
  switch(N){
    case 1:{
      uint8_t *syncword = (uint8_t *)&header_[64*N];
      mask = syncword[0] & syncword[15] & syncword[31]; 
      break;
    }
    case 2:{
      uint16_t *syncword = (uint16_t *)&header_[64*N];
      mask = syncword[0] & syncword[15] & syncword[31]; 
    }
    case 4:{
      uint32_t *syncword = (uint32_t *)&header_[64*N];
      mask = syncword[0] & syncword[15] & syncword[31]; 
      break;
    }
    case 8:{
      uint64_t *syncword = (uint64_t *)&header_[64*N];
      mask = syncword[0] & syncword[15] & syncword[31]; 
      break;
    }
  }
  return mask;
}


int Mark5a_header::nTracks() {
  return 8*N;
}


int Mark5a_header::year(int track) {
  SFXC_ASSERT((track >= 0) && (track < (int)N*8));
  return 2000 + BCD(96+ 0, track);
}


int Mark5a_header::day(int track) {
  return BCD(96+ 4, track)*100 +
         BCD(96+ 8, track)*10 +
         BCD(96+12, track);
}


int Mark5a_header::hour(int track) {
  return BCD(96+16, track)*10 + BCD(96+20, track);
}


int Mark5a_header::minute(int track) {
  return BCD(96+24, track)*10 + BCD(96+28, track);
}


int Mark5a_header::second(int track) {
  return BCD(96+32, track)*10 + BCD(96+36, track);
}


int Mark5a_header::milisecond(int track) {
  int unit = BCD(96+48, track);
  SFXC_ASSERT(unit != 4);
  SFXC_ASSERT(unit != 9);
  return BCD(96+40, track)*100 + BCD(96+44, track)*10 + unit;
}

int Mark5a_header::microsecond(int track, int milisec) {
  // See table 2 of Mark IIIA/IV/VLBA tape format specs(rev 1.21), whitney 2005
  int unit = milisec%10;
  SFXC_ASSERT(unit != 4);
  SFXC_ASSERT(unit != 9);
  return microsecond_offset[unit];
}


int Mark5a_header::headstack(int track) {
  return 2*get_bit(32,track) + get_bit(33,track);
}

int Mark5a_header::track(int track) {
  return
    10*(2*get_bit(34,track) + get_bit(35,track)) +
    BCD(36,track);
}

bool Mark5a_header::is_sign(int track) {
  return (get_bit(41, track) == 0);
}

bool Mark5a_header::is_magn(int track) {
  return (get_bit(41, track) == 1);
}


int Mark5a_header::find_track(int headstack_, int track_) {
  for (int i=0; i<nTracks(); i++) {
    if ((headstack(i) == headstack_) && (track(i) == track_)) {
      return i;
    }
  }
  //sfxc_abort("Couldn't find (headstack, track) in the mark5a header.");
  std::cout << RANK_OF_NODE << " : Warning - couldn't find headstack="<<headstack_
            << ", track="<< track_ << " in mark5a header\n";
  return -1;
}



bool Mark5a_header::checkCRC(uint8_t mask) {
  unsigned char crcBlock[12*N];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 12*N);

  /* Re-calc CRC12. Buffer is 148 bits in size, CRC is 12 */
  switch (N) {
  case 1: {
      crc12((int8_t *)crcBlock, (int8_t *)header_, 160, (int8_t)mask);
      break;
    }
  case 2: {
      int16_t mask_ = (mask << 8) | mask;
      crc12((int16_t*)crcBlock, (int16_t*)header_, 160, mask_);
      break;
    }
  case 4: {
      int32_t mask_ = mask;
      for(int i = 1; i<4; i++)
        mask_|= (mask << i*8);
      crc12((int32_t*)crcBlock, (int32_t*)header_, 160, mask_);
      break;
    }
  case 8: {
      int64_t mask_ = mask;
      for(int i = 1; i<8; i++)
        mask_|= (mask << i*8);
      crc12((int64_t*)crcBlock, (int64_t*)header_, 160, mask_);
      break;
    }
  }

  for (size_t i=0; i<12*N; i++) {
    if (crcBlock[i] != 0) {
      std::cout << "Error in crc " << i << " "
      << std::hex << crcBlock[i] << std::dec
      << std::endl;
      return false;
    }
  }

  return true;
}


void Mark5a_header::recomputeCRC(uint8_t mask) {
  unsigned char crcBlock[12];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 12*N);

  /* Re-calc CRC12. Buffer is 148 bits in size, CRC is 12 */
  switch (N) {
  case 1: {
      crc12((int8_t*)crcBlock, (int8_t*)header_, 160, (int8_t)mask);
      break;
    }
  case 2: {
      int16_t mask_ = (mask << 8) | mask;
      crc12((int16_t*)crcBlock, (int16_t*)header_, 160, mask_);
      break;
    }
  case 4: {
      int32_t mask_ = mask;
      for(int i = 1; i<4; i++)
        mask_|= (mask << i*8);
      crc12((int32_t*)crcBlock, (int32_t*)header_, 160, mask_);
      break;
    }
  case 8: {
      int64_t mask_ = mask;
      for(int i = 1; i<8; i++)
        mask_|= (mask << i*8);
      crc12((int64_t*)crcBlock, (int64_t*)header_, 160, mask_);
      break;
    }
  }

  for (int i=0; i<12; i++) {
    for (size_t j=0; j<N; j++) {
      header_[(148+i)*N+j] = crcBlock[(11-i)*N+j];
    }
  }

  checkCRC(mask);
}


template <class Type>
void Mark5a_header::crc12(Type *crcBlock,
                         Type *data,
                         int datawords, Type mask) {
  SFXC_ASSERT(sizeof(Type) == N);
  SFXC_ASSERT(Type(-1)^Type(0) == Type(-1));
//  for (size_t i=64; i<96; i++) {
//    SFXC_ASSERT(data[i] == Type(-1));
//  }
  /* 'cr' CRC register is assumed to be initialized */
  /* with 12 data words. */
  /* (Init to all zero; or use old regs to continue. */
  for (int i=0; i<datawords; i++) {
    Type n0  = ((*data++) & mask) ^ crcBlock[11];  /* x^12 + */
    Type n11 = n0 ^ crcBlock[10];  /* x^11 + */
    Type n3  = n0 ^ crcBlock[2];   /* x^3 + */
    Type n2  = n0 ^ crcBlock[1];   /* x^2 + */
    Type n1  = n0 ^ crcBlock[0];   /* x + 1 */

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


int
Mark5a_header::BCD(int word, unsigned int track) {
  SFXC_ASSERT(track<8*N);
  int result = get_bit(word,track)*8 +
               get_bit(word+1,track)*4 +
               get_bit(word+2,track)*2 +
               get_bit(word+3,track);
  if (result >= 10) {
    std::cout << "BCD >= 10" << std::endl;
  }
  return result;
}


int
Mark5a_header::get_time_in_ms(int track) {
  int result =
    milisecond(track) +
    1000*(second(track) +
          60 * (minute(track) +
                60*(int64_t)hour(track)));
  //   DEBUG_MSG("time: " << result);
  return result;
}


int64_t
Mark5a_header::get_time_in_us(int track) {
  int64_t time_in_ms = get_time_in_ms(track);
  return 1000*time_in_ms + microsecond(track, time_in_ms);
}


std::string
Mark5a_header::get_time_str(int track) {
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


Log_writer &
Mark5a_header::print_binary_header(Log_writer &writer) {
  for (int i=0; i<180; i++) {
    for (size_t track=0; track<N*8; track++) {
      writer << get_bit(i,track);
    }
    writer << " " << i << " " << header_map[i] << std::endl;
  }

  return writer;
}



const char *Mark5a_header::header_map[] = {
      "0hp15",
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
      ".dat."
    };
