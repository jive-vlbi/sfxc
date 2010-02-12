#include <cstring>
#include "vlba_header.h"
#include "backtrace.h"

VLBA_header::VLBA_header(int N_) : header(NULL), header_aux(NULL), N(N_) {
  SFXC_ASSERT(N_ > 0);
}

void VLBA_header::set_header(unsigned char* hdr, unsigned char* aux) {
  if ((hdr == NULL) ||(aux==NULL))
    std::cout << "Backtrace: " << Backtrace() << std::endl;
  SFXC_ASSERT((hdr != NULL)&&(aux != NULL));
  header = hdr;
  header_aux = aux;
}

bool VLBA_header::check_header() {
  // Check for a valid header
  SFXC_ASSERT(header != NULL);
  if (!is_valid())
    return false;
  if (!checkCRC())
    return false;
  return true;
}


bool VLBA_header::is_valid() {
  for (size_t i=0*N; i<32*N; i++) {
    SFXC_ASSERT((unsigned char)(-1)^(unsigned char)(0) == (unsigned char)(-1));
    if (header[i] != (unsigned char)(-1)) {
//      char word[8*N];
//      for (int j=0*N; j<32*(int)N; j++) {
//        itoa(header[j], word, 2);
//        printf(" Word (vlba): %03d %16s\n", j, word);
//      }
      std::cout << "Invalid VLBA frame : No syncword found " << i << std::endl;
      return false;
    }
  }
  return true;
}

int VLBA_header::nTracks() {
  return 8*N;
}

int64_t VLBA_header::microseconds(int track)  {
  int64_t sec = seconds(track);
  int subsec1=BCD(32+32, track, header), subsec2 = BCD(32+36, track, header);
  int subsec3=BCD(32+40, track, header), subsec4 = BCD(32+44, track, header);
  
int64_t microseconds =
    (((((int64_t)subsec1)*10+subsec2)*10+subsec3)*10+subsec4)*100;
  return 1000000*sec + microseconds;
}

int64_t VLBA_header::seconds(int track)  {
  int sec1 = BCD(32+12, track, header), sec2 = BCD(32+16, track, header), sec3 = BCD(32+20, track, header);
  int sec4 = BCD(32+24, track, header), sec5 = BCD(32+28, track, header);
  return (((((int32_t)sec1*10 + sec2)*10+sec3)*10+sec4)*10+sec5);
}

int VLBA_header::julian_day(int track)  {
  int day1 = BCD(32+0, track, header), day2 = BCD(32+4, track, header), day3 = BCD(32+8, track, header);
  return ((((int32_t)day1)*10+day2)*10+day3);
}

int VLBA_header::headstack(int track) {
  return BCD(12, track, header_aux);
}

int VLBA_header::dar(int track) {
  return BCD_REVERSE(52, track, header_aux);
}

int VLBA_header::track(int track_) {
  int retval;
  int track_digit=BCD_REVERSE(32,track_, header_aux);
  int group_digit=BCD_REVERSE(36,track_, header_aux);
  
//the following is from Table 18 of the MARKIII/IV/VLBA TAPE FORMAT
//specifications, whitney2005 rev 1.21
  switch(group_digit){
  case 0:
    retval = 2*track_digit+3;
    break;
  case 2:
    retval = 2*track_digit+2;
    break;
  case 1:
    retval = 2*track_digit+19;
    break;
  case 3:
    retval = 2*track_digit+18;
    break;
  case 4:
    switch(track_digit){
      case 0:
        retval=1;
        break;
      case 1:
        retval=35;
        break;
      case 2:
        retval=0;
        break;
      case 3:
        retval=34;
        break;
      default:
          std::cerr << "illegal track digit : " << track_digit << std::endl;
          sfxc_abort(); // TODO: trigger resync
      }
      break;
    default:
          std::cerr << "illegal group digit : " << group_digit << std::endl;
          sfxc_abort(); // TODO: trigger resync
  }
  return retval;
}

bool VLBA_header::is_sign(int track) {
  return (get_bit(40, track, header_aux) == 0);
}

bool VLBA_header::is_magn(int track) {
  return (get_bit(40, track, header_aux) == 1);
}


int VLBA_header::find_track(int headstack_, int track_) {
  for (int i=0; i<nTracks(); i++) {
    // the headstack field of the header is not written reliably
    // instead the DAR field is used to locate tracks
    if ((dar(i) == headstack_) && (track(i) == track_)) {
      return i;
    }
  }
  sfxc_abort("Couldn't find (headstack, track) in the vlba header.");
  return -1;
}


bool VLBA_header::checkCRC() {
  unsigned char crcBlock[16*N];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 16*N);

   /* Re-calc CRC16. Buffer is 48 bits in size, CRC is 16 */
  switch (N) {
  case 1: {
      int8_t *header_i8= (int8_t *)header;
      crc16((int8_t *)crcBlock, &header_i8[32], 64);
      break;
    }
  case 2: {
      int16_t *header_i16= (int16_t *)header;
      crc16((int16_t*)crcBlock, &header_i16[32], 64);
      break;
    }
  case 4: {
      int32_t *header_i32= (int32_t *)header;
      crc16((int32_t*)crcBlock, &header_i32[32], 64);
      break;
    }
  case 8: {
      int64_t *header_i64= (int64_t *)header;
      crc16((int64_t*)crcBlock,&header_i64[32], 64);
      break;
    }
  }

  for (size_t i=0; i<16*N; i++) {
    if (crcBlock[i] != 0) {
      std::cout << "Error in crc " << i << " "
      << std::hex << crcBlock[i] << std::dec
      << std::endl;
      return false;
    }
  }

  return true;
}


void VLBA_header::recomputeCRC() {
  unsigned char crcBlock[16];

  /* Init CRC generator to all zeroes. */
  memset(crcBlock, 0, 16*N);

  /* Re-calc CRC16. Buffer is 48 bits in size, CRC is 16 */
  switch (N) {
  case 1: {
      int8_t *header_i8= (int8_t *)header;
      crc16((int8_t *)crcBlock, &header_i8[32], 64);
      break;
    }
  case 2: {
      int16_t *header_i16= (int16_t *)header;
      crc16((int16_t*)crcBlock, &header_i16[32], 64);
      break;
    }
  case 4: {
      int32_t *header_i32= (int32_t *)header;
      crc16((int32_t*)crcBlock, &header_i32[32], 64);
      break;
    }
  case 8: {
      int64_t *header_i64= (int64_t *)header;
      crc16((int64_t*)crcBlock,&header_i64[32], 64);
      break;
    }
  }

  for (int i=0; i<16; i++) {
    for (size_t j=0; j<N; j++) {
      header[(48+i)*N+j] = crcBlock[(15-i)*N+j];
    }
  }

  checkCRC();
}


template <class Type>
void VLBA_header::crc16(Type *crcBlock,
                         Type *data,
                         int datawords) {
  SFXC_ASSERT(sizeof(Type) == N);
  SFXC_ASSERT(Type(-1)^Type(0) == Type(-1));

  /* 'cr' CRC register is assumed to be initialized */
  /* with 16 data words. */
  /* (Init to all zero; or use old regs to continue. */
  for (int i=0; i<datawords; i++) {
    Type n0  = (*data++) ^ crcBlock[15];  /* x^16 + */
    Type n15 = n0 ^ crcBlock[14];  /* x^15 + */
    Type n2  = n0 ^ crcBlock[1];   /* x^2 + */

    crcBlock[15] = n15;
    for(int j=14;j>2;j--)
      crcBlock[j] = crcBlock[j-1];
    crcBlock[ 2] = n2;
    crcBlock[ 1] = crcBlock[ 0];
    crcBlock[ 0] = n0;
  }
} 


int
VLBA_header::BCD(int word, unsigned int track, unsigned char *header_) {
  SFXC_ASSERT(track<8*N);
  int result = get_bit(word,track,header_)*8 +
               get_bit(word+1,track,header_)*4 +
               get_bit(word+2,track,header_)*2 +
               get_bit(word+3,track,header_);
  if (result >= 10) {
    std::cout << "BCD >= 10" << std::endl;
  }
  return result;
}


int
VLBA_header::BCD_REVERSE(int word, unsigned int track, unsigned char *header_) {
  SFXC_ASSERT(track<8*N);
  int result = get_bit(word,track,header_) +
               get_bit(word+1,track,header_)*2 +
               get_bit(word+2,track,header_)*4 +
               get_bit(word+3,track,header_)*8;
  if (result >= 10) {
    std::cout << "BCD >= 10" << std::endl;
  }
  return result;
}
