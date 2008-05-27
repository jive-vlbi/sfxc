#include "mark5b_reader.h"

Mark5b_reader::
Mark5b_reader(boost::shared_ptr<Data_reader> data_reader)
    : data_reader_(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS) {
  assert(sizeof(current_header) == SAMPLES_PER_HEADER*WORD_SIZE);
  data_reader_->get_bytes(sizeof(current_header), (char *)&current_header);

  // Don't count the first header
  data_reader_->reset_data_counter();

  assert(current_header.check());
  start_day_ = current_header.julian_day();
  start_time_ = current_header.microseconds();
  current_time_ = start_time_;

  std::cout << current_header.microseconds() << std::endl;

}


Mark5b_reader::~Mark5b_reader() {}

int64_t
Mark5b_reader::goto_time(unsigned char *mark5b_block, int64_t us_time) {
  if (us_time <= current_time_) return current_time_;

  if (current_time_ == start_time_) {
    // Make sure we can compute the data rate
    read_new_block(mark5b_block);

    if (us_time <= current_time_) return current_time_;
  }

  int64_t bytes_read = data_reader_->data_counter();
  int64_t delta_time = current_time_-start_time_;
  
  // Only read an integer number of blocks:
  assert((bytes_read*(us_time-current_time_)) % 
         (delta_time*(SAMPLES_PER_HEADER+SAMPLES_PER_BLOCK)*WORD_SIZE) == 0);

  data_reader_->get_bytes((bytes_read*(us_time-current_time_)) / delta_time -
                          (SAMPLES_PER_HEADER+SAMPLES_PER_BLOCK)*WORD_SIZE,
                          NULL);

  read_new_block(mark5b_block);

  return current_time_;
}

int64_t Mark5b_reader::get_current_time() {
  return current_time_;
}


bool Mark5b_reader::read_new_block(unsigned char *mark5b_block) {
  data_reader_->get_bytes(2500*sizeof(uint32_t), (char *)mark5b_block);
  
  data_reader_->get_bytes(sizeof(current_header), (char *)&current_header);

  current_time_ = current_header.microseconds();
  
  return current_header.check();
}

bool Mark5b_reader::eof() {
  return data_reader_->eof();
}

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         unsigned char first_block[]) {
  // first_block is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the first_block and then look for the header
  // if we don't find a header, read in another half block and continue.
  size_t bytes_to_read = SIZE_MK4_FRAME/2;
  char *data = (char *)first_block+SIZE_MK4_FRAME/2;
  do {
    int read = reader->get_bytes(bytes_to_read, data);
    bytes_to_read -= read;
    data += read;
  } while (bytes_to_read > 0);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(first_block, first_block+SIZE_MK4_FRAME/2, SIZE_MK4_FRAME/2);

    size_t bytes_to_read = SIZE_MK4_FRAME/2;
    char *data = (char*)first_block+SIZE_MK4_FRAME/2;
    do {
      int read = reader->get_bytes(bytes_to_read, data);
      bytes_to_read -= read;
      data += read;
    } while (bytes_to_read > 0);


    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if ((char)first_block[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if (nOnes>=32) {
          // make sure the begin of the header is in the first_block
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes - 64*(nOnes/32);
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;

            memmove(first_block, first_block+header_start,
                    SIZE_MK4_FRAME-header_start);
            reader->get_bytes(header_start,
                              (char *)first_block+SIZE_MK4_FRAME-header_start);

            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}

template <class T>
std::ostream &print_binary(std::ostream &out, const T &data, 
                           int len=sizeof(T)*8) {
  for (int i=len-1; i>=0; i--) {
    out << ((data>>i)&1);
    if ((i>0) && (!(i%8))) out << " ";
  }
  return out;
}
 
std::ostream &
operator<<(std::ostream &out,
           const Mark5b_reader::Header &h) {
  out << "syncword: ";
  print_binary(out, h.syncword);
  out << std::endl;;
  out << "frame nr: ";
  print_binary(out, (uint16_t)h.frame_nr,15);
  out << std::endl;
  out << "tvg:      " << (int)h.tvg << std::endl;
  out << "user spec:";
  print_binary(out, (uint16_t)h.user_specified);
  out << std::endl;

  out << "year:     ";
  print_binary(out, (uint16_t)h.day1,4) << " ";
  print_binary(out, (uint16_t)h.day2,4) << " ";
  print_binary(out, (uint16_t)h.day3,4) << " ";
  out << "[" << (uint16_t)h.day1 << (uint16_t)h.day2 << (uint16_t)h.day3 <<"]";
  out << std::endl;
  out << "sec:      ";
  print_binary(out, (uint16_t)h.sec1,4) << " ";
  print_binary(out, (uint16_t)h.sec2,4) << " ";
  print_binary(out, (uint16_t)h.sec3,4) << " ";
  print_binary(out, (uint16_t)h.sec4,4) << " ";
  print_binary(out, (uint16_t)h.sec5,4) << " ";
  out << "[" << (uint16_t)h.sec1 << (uint16_t)h.sec2 << (uint16_t)h.sec3
      << (uint16_t)h.sec4
      << (uint16_t)h.sec5 <<"]";
  out << std::endl;

  out << "crc:      ";
  print_binary(out, (uint16_t)h.crc) << " ";
  out << std::endl;

  out << "sub sec:  ";
  print_binary(out, (uint16_t)h.subsec1,4) << " ";
  print_binary(out, (uint16_t)h.subsec2,4) << " ";
  print_binary(out, (uint16_t)h.subsec3,4) << " ";
  print_binary(out, (uint16_t)h.subsec4,4) << " ";
  out << "[" << (uint16_t)h.subsec1 << (uint16_t)h.subsec2 << (uint16_t)h.subsec3
      << (uint16_t)h.subsec4 <<"]";
  out << std::endl;

  return out;
}

bool Mark5b_reader::Header::check() const {
  if (syncword != 0xABADDEED) 
    return false;

  return true;
}

int64_t Mark5b_reader::Header::microseconds() const {
  int seconds = 
    (((((int32_t)sec1*10 + sec2)*10+sec3)*10+sec4)*10+sec5);
  int microseconds = 
    (((((int32_t)subsec1)*10+subsec2)*10+subsec3)*10+subsec4)*100;
  return int64_t(1000000)*seconds + microseconds;
}

int Mark5b_reader::Header::julian_day() const {
  return ((((int32_t)day1)*10+day2)*10+day3);
}
