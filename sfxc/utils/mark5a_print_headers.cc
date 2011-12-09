/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include <iostream>

#include "log_writer_cout.h"
#include "data_reader_file.h"
#include "data_reader_blocking.h"

#include "utils.h"
#include "input_node_types.h"
#include "mark5a_reader.h"

typedef Input_node_types::Data_memory_pool Data_memory_pool;
int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         Mark5a_reader::Data_frame &data) {
  // We fill the "data" and then look for the header
  // if we don't find a header, read in another half block and continue.

  data.buffer->data.resize(SIZE_MK5A_FRAME);
  char *buffer_start = (char *)&data.buffer->data[0];

  { // Read half a block
    size_t bytes_to_read = SIZE_MK5A_FRAME/2;
    char *data = (char *)buffer_start+SIZE_MK5A_FRAME/2;

    int byte_read = Data_reader_blocking::get_bytes_s( reader.get(), bytes_to_read, data);

    if( byte_read != bytes_to_read ){
      std::cout << "Unable to read enough bytes of data, cannot find a mark5a header before the end-of-file\n";
      exit(1);
    }
  }

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read frameMk5a/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_MK5A_FRAME/2, SIZE_MK5A_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_MK5A_FRAME/2;
      char *data = (char*)buffer_start+SIZE_MK5A_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(), bytes_to_read, data);

    }

    // the header contains 64 bits before the syncword and
    //                     64 bits after the syncword.
    // We skip those bytes since we want to find an entire syncword
    for (int byte=0; (byte<SIZE_MK5A_FRAME-64*8) && (header_start<0); byte++) {
      if ((char)buffer_start[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        if (nOnes>=32) {
          // make sure the begin of the header is in the first_block
          // syncword is 32 samples, auxiliary data field 64 samples
          header_start = byte - nOnes - 64*(nOnes/32);
          if (header_start >= 0) {
            // We found a complete header
            nTracks8 = nOnes/32;

            memmove(buffer_start, buffer_start+header_start,
                    SIZE_MK5A_FRAME-header_start);

            int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                     header_start,
                                     buffer_start+SIZE_MK5A_FRAME-header_start);

            if (nTracks8 > 1) {
              data.buffer->data.resize(nTracks8*SIZE_MK5A_FRAME);
              data.buffer->data.resize(nTracks8*SIZE_MK5A_FRAME);
              buffer_start = (char *)&data.buffer->data[0];

              int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                                  (nTracks8-1)*SIZE_MK5A_FRAME,
                                                   buffer_start+SIZE_MK5A_FRAME);
            }
            return nTracks8;
          }
        }
        nOnes=0;
      }
    }
  }
  return -1;
}

void get_input_node_parameters(Input_node_parameters &param, Mark5a_reader::Data_frame data, char *filename)
// Get a subset of the input node parameter which are needed by 
// the mark5a reader to read the timestamps in the datastream
{
  int n_tracks_8;
  bool header_correct=false;
  bool first_msg=true;
  boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
  do{
    n_tracks_8 = find_start_of_header(reader, data);
    if(n_tracks_8 > 0){
            Mark5a_header header(n_tracks_8);
            header.set_header(&data.buffer->data[0]);
            header_correct = header.checkCRC(0xff);
            if((first_msg)&&(!header_correct)){
              std::cout << RANK_OF_NODE
                        << " Warning : Invalid crc-code in the mark5a data file, further warnings are supressed.\n";
              first_msg=false;
            }
    }
  }while(!header_correct);
  param.n_tracks = n_tracks_8 * 8;
  param.track_bit_rate = 1;
}

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  Log_writer_cout log_writer(0);

  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " <mark5a-file>" << std::endl;
    return 1;
  }

  Mark5a_reader::Data_frame data;
  boost::shared_ptr< Data_memory_pool > memory_pool_(new Data_memory_pool(10));
  data.buffer = memory_pool_->allocate();

  Input_node_parameters param;
  get_input_node_parameters(param, data, argv[1]);

  boost::shared_ptr<Data_reader> reader(new Data_reader_file(argv[1]));
  Mark5a_reader *mark5a_reader = new Mark5a_reader(reader, Time(0));
  mark5a_reader->set_parameters(param);

  while ((!mark5a_reader->open_input_stream(data)) && (!mark5a_reader->eof()))
    ;
  int64_t prev_time = (int64_t)mark5a_reader->get_current_time().get_time_usec(), current_time;

  do {
    current_time = (int64_t)mark5a_reader->get_current_time().get_time_usec();
    std::cout
    << current_time
    << " \t" << current_time - prev_time
    << std::endl;
    prev_time = current_time;
  } while (mark5a_reader->read_new_block(data));

}
