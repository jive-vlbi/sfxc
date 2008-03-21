#include "mark4_reader.h"
#include "mark4_header.h"

Mark4_reader_interface *
get_mark4_reader(boost::shared_ptr<Data_reader> reader,
                 char *first_block) {

  int n_tracks_8 = find_start_of_header(reader, first_block);
  switch (n_tracks_8) {
    case 1: {
      Mark4_header<uint8_t> header;
      header.set_header((uint8_t*)(first_block));
      if (!header.checkCRC()) {
        assert(false);
        return NULL;
      }
      return new Mark4_reader<int8_t>(reader, first_block,
                                      (int8_t*)first_block);
    }
    case 2: {
      Mark4_header<uint16_t> header;
      header.set_header((uint16_t*)(first_block));
      if (!header.checkCRC()) {
        assert(false);
        return NULL;
      }
      return new Mark4_reader<int16_t>(reader, first_block,
                                       (int16_t*)first_block);
    }
    case 4: {
      Mark4_header<uint32_t> header;
      header.set_header((uint32_t*)(first_block));
      if (!header.checkCRC()) {
        assert(false);
        return NULL;
      }
      return new Mark4_reader<int32_t>(reader, first_block,
                                       (int32_t*)first_block);
    }
    case 8: {
      Mark4_header<uint64_t> header;
      header.set_header((uint64_t*)(first_block));
      if (!header.checkCRC()) {
        assert(false);
        return NULL;
      }
      return new Mark4_reader<int64_t>(reader, first_block,
                                       (int64_t*)first_block);
    }
    default: {
      assert(false);
    }
  }
  return NULL;
}

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         char first_block[]) {
  // first_block is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the first_block and then look for the header
  // if we don't find a header, read in another half block and continue.
  size_t bytes_to_read = SIZE_MK4_FRAME/2;
  char *data = first_block+SIZE_MK4_FRAME/2;
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
    char *data = first_block+SIZE_MK4_FRAME/2;
    do {
      int read = reader->get_bytes(bytes_to_read, data);
      bytes_to_read -= read;
      data += read;
    } while (bytes_to_read > 0);


    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if (first_block[byte] == (char)(~0)) {
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
                              first_block+SIZE_MK4_FRAME-header_start);

            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}
