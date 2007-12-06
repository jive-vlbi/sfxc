#include "input_node_tasklet.h"
#include "input_node_tasklet_impl.h"
#include "utils.h"
#include "mark4_reader.h"

Input_node_tasklet::Input_node_tasklet() {
}

Input_node_tasklet::~Input_node_tasklet() {
}

Input_node_tasklet *
get_input_node_tasklet(Data_reader *reader) {
  char buffer[SIZE_MK4_FRAME];

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
            case 1:
              {
                Mark4_header<uint8_t> header;
                header.set_header((uint8_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                Mark4_reader<int8_t> *mark4_reader = 
                  new Mark4_reader<int8_t>(reader, buffer);
                return new Input_node_tasklet_implementation<int8_t>(mark4_reader);
              }
            case 2:
              {
                Mark4_header<uint16_t> header;
                header.set_header((uint16_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                Mark4_reader<int16_t> *mark4_reader = 
                  new Mark4_reader<int16_t>(reader, buffer);
                return new Input_node_tasklet_implementation<int16_t>(mark4_reader);
              }
            case 4:
              {
                Mark4_header<uint32_t> header;
                header.set_header((uint32_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                Mark4_reader<int32_t> *mark4_reader = 
                  new Mark4_reader<int32_t>(reader, buffer);
                return new Input_node_tasklet_implementation<int32_t>(mark4_reader);
              }
            case 8:
              {
                Mark4_header<uint64_t> header;
                header.set_header((uint64_t*)(buffer));
                if (!header.checkCRC()) {
                  header_start = -1;
                }
                Mark4_reader<int64_t> *mark4_reader = 
                  new Mark4_reader<int64_t>(reader, buffer);
                return new Input_node_tasklet_implementation<int64_t>(mark4_reader);
              }
            default:
              {
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

bool 
Input_node_tasklet::append_time_slice(int start_time,
                                      int stop_time,
                                      Data_writer *writer) {
  return append_time_slice(Time_slice(start_time, stop_time, writer));
}
  
Input_node_tasklet::Time_slice::Time_slice() {
  
}
Input_node_tasklet::Time_slice::
Time_slice(int start_time, int stop_time, Data_writer *writer) 
: start_time(start_time), stop_time(stop_time), writer(writer) {
  
}
