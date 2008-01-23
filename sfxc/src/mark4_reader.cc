#include "mark4_reader.h"

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         char buffer[]) {
  // PRE: buffer is of size SIZE_MK4_FRAME
  assert(SIZE_MK4_FRAME%2 == 0);

  // buffer is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the buffer and then look for the header
  // if we don't find a header, read in another half block and continue.

  size_t bytes_read = reader->get_bytes(SIZE_MK4_FRAME/2, buffer+SIZE_MK4_FRAME/2);
  assert (bytes_read == SIZE_MK4_FRAME/2);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(buffer, buffer+SIZE_MK4_FRAME/2, SIZE_MK4_FRAME/2);
    size_t bytes_read = reader->get_bytes(SIZE_MK4_FRAME/2, buffer+SIZE_MK4_FRAME/2);
    assert (bytes_read == SIZE_MK4_FRAME/2);

    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if (buffer[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if ((nOnes>0) && (nOnes%32 == 0)) {
          // make sure the begin of the header is in the buffer
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes*3;
          if (header_start >= 0) {
            // We found a complete header
            assert((nOnes % 32) == 0);
            nTracks8 = nOnes/32;

            memmove(buffer, buffer+header_start, SIZE_MK4_FRAME-header_start);
            reader->get_bytes(header_start, buffer+SIZE_MK4_FRAME-header_start);

            switch (nTracks8) {
              case 1: {
                Mark4_header<uint8_t> header;
                header.set_header((uint8_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                } else return nTracks8;
                break;
              }
              case 2: {
                Mark4_header<uint16_t> header;
                header.set_header((uint16_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                } else return nTracks8;
                break;
              }
              case 4: {
                Mark4_header<uint32_t> header;
                header.set_header((uint32_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                } else return nTracks8;
                break;
              }
              case 8: {
                Mark4_header<uint64_t> header;
                header.set_header((uint64_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                } else return nTracks8;
                break;
              }
              default: {
                assert(false);
              }
            }
          }
        }
        nOnes=0;
      }
    }
  }

  assert(false);
  return -1;
}

