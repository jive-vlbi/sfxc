/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#include "input_node_tasklet.h"
#include "input_node_tasklet_impl.h"
#include "utils.h"
#include "mark4_reader.h"

Input_node_tasklet::Input_node_tasklet() {}

Input_node_tasklet::~Input_node_tasklet() {}

Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader) {
  assert(SIZE_MK4_FRAME%2 == 0);
  char buffer[SIZE_MK4_FRAME];

  // buffer is an array of SIZE_MK4_FRAME bytes (8 is the smallest number of tracks).
  // We fill the buffer and then look for the header
  // if we don't find a header, read in another half block and continue.
  char * data = buffer+SIZE_MK4_FRAME/2;
  int bytes_to_read = SIZE_MK4_FRAME/2;
  do {
    int bytes_read = reader->get_bytes(bytes_to_read, data);
    bytes_to_read -= bytes_read;
    data += bytes_read;
    assert (bytes_read >= 0);
  } while (bytes_to_read > 0);

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk4/2 bytes:
    memcpy(buffer, buffer+SIZE_MK4_FRAME/2, SIZE_MK4_FRAME/2);
    char * data = buffer+SIZE_MK4_FRAME/2;
    int bytes_to_read = SIZE_MK4_FRAME/2;
    do {
      int bytes_read = reader->get_bytes(bytes_to_read, data);
      bytes_to_read -= bytes_read;
      data += bytes_read;
      assert (bytes_read >= 0);
    } while (bytes_to_read > 0);

    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK4_FRAME-64*8) && (header_start<0); byte++) {
      if (buffer[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if (nOnes>=32) {
          // make sure the begin of the header is in the buffer
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes - 64*(nOnes/32);
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;

            memmove(buffer, buffer+header_start,
                    SIZE_MK4_FRAME-header_start);
            reader->get_bytes(header_start,
                              buffer+SIZE_MK4_FRAME-header_start);

            Mark4_header header(nTracks8);
            header.set_header((unsigned char *)buffer);
            assert(header.checkCRC());

            switch (nTracks8) {
              case 1: {
                return new Input_node_tasklet_implementation<int8_t>(reader, buffer);
              }
              case 2: {
                return new Input_node_tasklet_implementation<int16_t>(reader, buffer);
              }
              case 4: {
                return new Input_node_tasklet_implementation<int32_t>(reader, buffer);
              }
              case 8: {
                return new Input_node_tasklet_implementation<int64_t>(reader, buffer);
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

  return NULL;
}
