/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: test_Input_node.cc 285 2007-07-18 07:15:38Z kruithof $
 *
 *  Tests the channel extractor.
 */

#include <Control_parameters.h>
#include <Channel_extractor_mark4.h>
#include <Data_reader_file.h>
#include <Data_writer_file.h>
#include <utils.h>

class Channel_extractor_tester {
public:
  Channel_extractor_tester(const std::string &filename, 
                           Track_parameters &parameters);

  void writer(int channel, const std::string &filename);
  
  void goto_time(int64_t time);
  void write_block();
private:
  boost::shared_ptr<Channel_extractor_mark4> channel_extractor;
  Track_parameters track_parameters;
  std::vector< boost::shared_ptr<Data_writer> >   output_writers;
  
  // fanout is at most 8/n_bits_per_sample
  std::vector< char* > buffer;
};


/** Main **/
int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0; 
#endif
  assert(argc==3);

  Control_parameters control_parameters;
  control_parameters.initialise(argv[1], argv[2], std::cout);

  std::vector<std::string> scans;
  control_parameters.get_vex().get_scans(std::back_inserter(scans));
  // Get all stations for a scan 0
  const std::string & station = control_parameters.station(0);
  
  //use first station, first track
  const Vex & root = control_parameters.get_vex();
  const std::string &track = 
    root.get_track(control_parameters.get_vex().get_mode(scans[0]),
                   station);

  { // First test
    Track_parameters track_parameters = 
      control_parameters.get_track_parameters(track);

    Channel_extractor_tester 
      tester(control_parameters.data_sources(station)[0],
             track_parameters);

    std::string outfile0 = "file:///tmp/output_channel0_0.ch";
    std::string outfile1 = "file:///tmp/output_channel1_0.ch";
    tester.writer(0, outfile0);
    tester.writer(1, outfile1);

    tester.goto_time(control_parameters.get_start_time().to_miliseconds());

    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
  }

  { // Second test
    // Swap first two channels
    Track_parameters track_parameters = 
      control_parameters.get_track_parameters(track);
    Track_parameters::Channel_iterator first, second;
    first = track_parameters.channels.begin();
    second = first; second++;
    Track_parameters::Channel_parameters channel_parameters = first->second;
    first->second = second->second;
    second->second = channel_parameters;

    Channel_extractor_tester 
      tester(control_parameters.data_sources(station)[0],
             track_parameters);

    std::string outfile0 = "file:///tmp/output_channel1_1.ch";
    std::string outfile1 = "file:///tmp/output_channel0_1.ch";
    tester.writer(0, outfile0);
    tester.writer(1, outfile1);

    tester.goto_time(control_parameters.get_start_time().to_miliseconds());

    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
    tester.write_block();
  }

  if (system("cmp /tmp/output_channel0_0.ch /tmp/output_channel0_1.ch")) {
    std::cout << "cmp file output failed" << std::endl;
    return 1;
  }
  if (system("cmp /tmp/output_channel1_0.ch /tmp/output_channel1_1.ch")) {
    std::cout << "cmp file output failed" << std::endl;
    return 1;
  }
  
  return 0;
}

/** Channel_extractor_tester implementation **/
Channel_extractor_tester::
Channel_extractor_tester(const std::string &input_filename,
                         Track_parameters &parameters) 
  : track_parameters(parameters) {
  boost::shared_ptr<Data_reader> 
    reader(new Data_reader_file(input_filename.c_str()));

  channel_extractor = boost::shared_ptr<Channel_extractor_mark4>
    (new Channel_extractor_mark4(reader, 
                                 /*random headers*/false));
  channel_extractor->set_track_parameters(track_parameters);

  buffer.resize(track_parameters.channels.size());
  for (size_t i=0; i<track_parameters.channels.size(); i++) {
    buffer[i] = new char[channel_extractor->number_of_bytes_per_block()];
  }
  output_writers.resize(track_parameters.channels.size());
}

void 
Channel_extractor_tester::
writer(int channel, const std::string &filename) {
  // boost::shared_ptr<Data_writer>(new Data_writer_file(filename));
  assert ((size_t)channel < output_writers.size());
  output_writers[channel] = 
    boost::shared_ptr<Data_writer>(new Data_writer_file(filename.c_str()));
}


void 
Channel_extractor_tester::
goto_time(int64_t time) {
  channel_extractor->goto_time(time);
  channel_extractor->goto_time(time);
}

void 
Channel_extractor_tester::
write_block() {
  int size = channel_extractor->get_bytes(buffer);
  for (size_t i=0; i<output_writers.size(); i++) {
    if (output_writers[i] != boost::shared_ptr<Data_writer>()) {
      assert(size < 8*SIZE_MK4_FRAME);
      output_writers[i]->put_bytes(size, buffer[i]);
    }
  }
  // proceed to the next block
  channel_extractor->goto_next_block();
}
