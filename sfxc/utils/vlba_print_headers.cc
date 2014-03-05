/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: vlba_print_headers.cc 1292 2011-12-09 13:45:41Z keimpema $
 *
 */

#include <iostream>
#include "data_reader_file.h"
#include "data_reader_blocking.h"

#include "utils.h"
#include "input_node_types.h"
#include "vlba_reader.h"
#include "vlba_header.h"

typedef Input_node_types::Data_memory_pool Data_memory_pool;

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         VLBA_reader::Data_frame &data,
                         std::vector<unsigned char> &header) {
  // We fill the "data" and then look for the header
  // if we don't find a header, read in another half block and continue.
  data.buffer->data.resize(SIZE_VLBA_FRAME);
  char *buffer_start = (char *)&data.buffer->data[0];

  { // Read half a block
    size_t bytes_to_read = SIZE_VLBA_FRAME/2;
    char *data = (char *)buffer_start+SIZE_VLBA_FRAME/2;

    int byte_read = Data_reader_blocking::get_bytes_s( reader.get(), bytes_to_read, data);

    if( byte_read != bytes_to_read ){
      sfxc_abort("Unable to read enough bytes of data, cannot find a vlba header before the end-of-file");
    }
  }

  int nOnes=0, header_start=-1, nTracks8 = -1;
  for (int block=0; (block<16) && (header_start<0); block++) {
    // Move the last half to the first half and read size_vlba_frame/2 bytes:
    memcpy(buffer_start, buffer_start+SIZE_VLBA_FRAME/2, SIZE_VLBA_FRAME/2);

    { // Read half a block
      size_t bytes_to_read = SIZE_VLBA_FRAME/2;
      char *data = (char*)buffer_start+SIZE_VLBA_FRAME/2;

      int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(), bytes_to_read, data);
    }

    for (int byte=0; (byte<SIZE_VLBA_FRAME - SIZE_VLBA_HEADER*8 ) && (header_start<0); byte++) {
      if ((char)buffer_start[byte] == (char)(~0)) {
        nOnes ++;
      } else {
        // Because the syncword is 32 ones, we should find atleast 32 (more depending on #tracks)
        if (nOnes>=32) {
          // make sure that really found the start of the header
          int header_start=byte-nOnes;
          if(header_start>=0){
            // We found a complete header
            nTracks8 = nOnes/32;

            // Store the header and get the first track8 worth of data
            int ndata_read=SIZE_VLBA_FRAME-header_start-nTracks8*SIZE_VLBA_HEADER;
            header.resize(nTracks8*(SIZE_VLBA_HEADER+SIZE_VLBA_AUX_HEADER));
            memcpy(&header[nTracks8*SIZE_VLBA_AUX_HEADER], 
                   buffer_start+header_start, 
                   nTracks8*SIZE_VLBA_HEADER);
            memmove(buffer_start, buffer_start+header_start+nTracks8*SIZE_VLBA_HEADER, ndata_read);
            int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),SIZE_VLBA_FRAME-ndata_read,
                                                               (char*)&data.buffer->data[ndata_read]);

            // read the remaining data
            if (nTracks8 > 1) {
              data.buffer->data.resize(nTracks8*SIZE_VLBA_FRAME);

              buffer_start = (char *)&data.buffer->data[0];
              int bytes_read = Data_reader_blocking::get_bytes_s(reader.get(),
                                                  (nTracks8-1)*SIZE_VLBA_FRAME,
                                                   buffer_start+SIZE_VLBA_FRAME);
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

void get_input_node_parameters(Input_node_parameters &param, VLBA_reader::Data_frame data, char *filename)
// Get a subset of the input node parameter which are needed by 
// the vlba reader to read the timestamps in the datastream
{
  int n_tracks_8;
  bool header_correct=false;
  bool first_msg=true;
  boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
  do{
    std::vector<unsigned char> buffer;
    n_tracks_8 = find_start_of_header(reader, data, buffer);
    if(n_tracks_8 > 0){
      VLBA_header header;
      header.set_header(n_tracks_8, &buffer[0]);
      header_correct = header.checkCRC(0xff);
      if((first_msg)&&(!header_correct)){
         std::cout << RANK_OF_NODE
                   << " Warning : Invalid crc-code in the vlba data file, further warnings are supressed.\n";
         first_msg=false;
      }
    }
  }while((!header_correct) && (!reader->eof()));
  if(reader->eof()){
    std::cerr << "Error finding header before EOF\n";
    exit(1);
  }

  param.n_tracks = n_tracks_8 * 8;
  param.track_bit_rate = 1;
}

void usage(char *filename){
  std::cout << "Usage : " << filename << " [OPTIONS] <filename>\n"
            << "Options : -n <number>, Only print <number> timestamps\n"
            << "          -y <YEAR>, the year in which the recording was made\n";
}

void parse_arguments(int argc, char *argv[], char **filename, int *n_time_stamps, int *year){
  int c;
  *n_time_stamps = -1;
  *year = 0;
    
  while ((c = getopt (argc, argv, "n:y:")) != -1){
    bool error = false;
    char *next;

    switch (c){
    case 'n':
      next = optarg;
      *n_time_stamps = strtol(optarg, &next, 10);
      error = (next == optarg);
     break;
    case 'y':
      next = optarg;
      *year = strtol(optarg, &next, 10);
      error = (next == optarg);
      break;
    case '?':
      std::cerr << "Error : Invalid option, " << (char)optopt << "\n";
      usage(argv[0]);
      exit(1);
    }
    if (error){
      std::cerr << "Error : invalid parameter\n";
      exit(1);
    }
  }
  if(argc - optind != 1){
    std::cerr << "Invalid number of arguments\n";
    usage(argv[0]);
    exit(1);
  }
  *filename = argv[optind];
}

boost::shared_ptr< Data_memory_pool > memory_pool_(new Data_memory_pool(10));

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif
  int n_time_stamps=-1, year = 0;
  char *filename; 

  parse_arguments(argc, argv, &filename, &n_time_stamps, &year);

  Time ref_time;
  if (year > 0){
    int y = year + 4799;
    int mjd_year = 365*y + (y/4) - (y/100) + (y/400) - 2431738.5; 
    ref_time.set_time(mjd_year, 0);
  }

  VLBA_reader::Data_frame data;
  data.buffer = memory_pool_->allocate();

  Input_node_parameters param;
  get_input_node_parameters(param, data, filename);

  boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
  VLBA_reader *vlba_reader = new VLBA_reader(reader, ref_time);
  vlba_reader->set_parameters(param);

  while ((!vlba_reader->open_input_stream(data)) && (!vlba_reader->eof()))
    ;
  int64_t prev_time = (int64_t)vlba_reader->get_current_time().get_time_usec(), current_time;

  int n = 0;
  do {
    current_time = (int64_t)vlba_reader->get_current_time().get_time_usec();
    std::cout
    << current_time
    << " \t" << vlba_reader->get_current_time()
    << " \t" << current_time - prev_time
    << std::endl;
    prev_time = current_time;
  } while((++n != n_time_stamps) && (vlba_reader->read_new_block(data)));
}
