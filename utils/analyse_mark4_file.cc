/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <iostream>
#include <../src/InData.cc>

#include <Log_writer_cout.h>
#include <Data_reader_file.h>
#include <Channel_extractor_mark4.h>

#include <utils.h>

uint32_t seed;


//*****************************************************************************
// find the header in the Mk4 type file after offset bytes and
// reset usEarliest in GemPrms if necessary
// input : station station number
// output: usTime  header time in us for requested offset
//         jsynch
//*****************************************************************************
int NGHK_FindHeaderMk4(Data_reader &reader, int& jsynch,
  int64_t& usTime, int64_t usStart, StaP &StaPrms, GenP &GenPrms)
{
  int retval = 0;
  
  //buffer for unpacked tracks, NTRACKS tracks, NFRMS Mk4 frames long
  char  tracks[trksMax][frameMk4*nfrms];
  int64_t jsynch0, jsynch1;
  int nhs, synhs1, synhs2;

  int Head0,Head1;  //Headstack IDs as seen in the header
  int year0,day0,hh0,mm0,ss0,ms0,us0; //TOT for headstack 0
  int year1,day1,hh1,mm1,ss1,ms1,us1; //TOT for headstack 1
  int64_t TOTusec0, TOTusec1; //in micro seconds
    
  //read and unpack scanfile data into tracks
  nhs = StaPrms.get_nhs();
  if (nhs==1) {
    if (read32datafile(reader, tracks) != 0) {
      get_log_writer()(0) << "Error in read32datafile." << std::endl;
      return -1;
    }
  } else {
    if (read64datafile(reader, tracks) != 0) {
      get_log_writer()(0) << "Error in read64datafile." << std::endl;
      return -1;
    }
  }

  //print track statistics on stdout
  if (get_log_writer().get_messagelevel()> 0)
    printTrackstats(tracks, nhs);
  
  //find sync word(s)
  synhs1 = StaPrms.get_synhs1();
  synhs2 = StaPrms.get_synhs2();
  if (findSyncWord(tracks, synhs1, 0,  &jsynch0) != 0)
    return -1;//no synchronisation word found
  if (nhs==2)
    if(findSyncWord(tracks, synhs2, 32, &jsynch1) != 0)
      return -1;//no synchronisation word found
  jsynch=jsynch0;
  
  // calculating TOT for headstack 0 
  timeComps(tracks, jsynch0, nhs, 0,
    &Head0, &year0, &day0, &hh0, &mm0, &ss0, &ms0, &us0, &TOTusec0);
    
  std::cout << "Head stack 0: " << TOTusec0 << std::endl;

  //set time for output in microseconds
  usTime = TOTusec0;
  
  if (nhs==2) {
    // calculating TOT for headstack 1
    timeComps(tracks, jsynch1, synhs2, 1,
      &Head1, &year1, &day1, &hh1, &mm1, &ss1, &ms1, &us1, &TOTusec1);

    std::cout << "Head stack 1: " << TOTusec1 << std::endl;
      
    if(jsynch0 != jsynch1) {
      get_log_writer()(0) << "\nWARNING: jsynch mismatch\n";
      return -1;
    }  
  }

  return retval;
  
}

int main(int argc, char *argv[]) {
  Log_writer_cout log_writer(0);
  
  if (argc != 4) {
    std::cout << "usage: " << argv[0]
              << " <vex-file> <ctrl-file> <station_name>" << std::endl;
    return 1;
  }

  int station_nr;
  str2int(argv[2], station_nr);

  Control_parameters control_parameters;
  control_parameters.initialise(argv[1], argv[2], log_writer);

  { // NGHK: TODO
    boost::shared_ptr<Data_reader> 
      data_reader(new Data_reader_file(staPrms[station_nr].get_mk4file()));
    Channel_extractor_mark4 
      ch_extractor(data_reader, 
                   /* insert_random_header */ false, 
                   Channel_extractor_mark4::CHECK_ALL_HEADERS);
    ch_extractor.print_header(log_writer(0), 0);
    for (int i=0; i<ch_extractor.n_tracks(); i++) {
      std::cout << "Track: " 
                << i << " \t"
                << ch_extractor.headstack(i) << " \t"
                << ch_extractor.track(i) << " \t"
                << std::endl;
    }

    int nBytes = 
      (frameMk4*staPrms[station_nr].get_fo()*staPrms[station_nr].get_bps())/8;
    char data_frame[nBytes];
    
    std::vector<char *> output;
    output.resize(
    for (int i=nBytes; i==nBytes; ) {
      i = ch_extractor.get_bytes(nBytes, data_frame);
      ch_extractor.goto_next_block();
    }
  }
}
