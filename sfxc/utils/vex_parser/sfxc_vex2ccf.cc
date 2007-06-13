//-----------------------------------------------------------------------------
//vex2ccf: a) program to extract some vex parameters
//         and write them into sfxc type correlator control file (ccf)
//         other ccf parameters are set to default values
//         manual editing of ccf required before correlation
//         b) this application also generates a control file for applicataion 
//         delmo which generates the appropriate delay model tables with the 
//         same files names as in the ccf
//using  : VexPlus class and the vex parser software
//author : RHJ Oerlemans
//date   : 29-01-2007
//-----------------------------------------------------------------------------

//change REVISION only if a keyword is changed, added or removed from cff
//do not forget the necessary changes in corresponding ccf parser code
#define REVISION 1.0

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

//c++ includes
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <iomanip>
using namespace std;

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "vexplus.h"
#include "json/json.h"
#include <set>

#include "../basic.h"


VexPlus vex_file;
Json::Value ctrl_file;
std::vector<int> stations;
              
// sets the vector stations
void set_stations();
void make_ccf(std::string const &channel1, std::string const &channel2);
void write_station_block(ofstream &cc_file, int nth_station_in_ccf, 
                         int station, int channel, int channel_for_delay_table,
                         std::string const &scan);

//MAIN
int main (int argc, char *argv[])
{

  if (argc != 3){
    cout << "purposes:a) create ccf for sfxc by extracting relevant\n";
    cout << "            vex parameters and setting defaults values\n";
    cout << "            for the others.\n";
    cout << "         b) create dcf and a local copy of DE405_le.jpl for \n";
    cout << "            application delmo.\n\n";
    cout << "usage  : vex2ccf <vexfilename> <sfxc-exp-ctrlfile>\n";
    return 0;
  }

  // Vex file
  vex_file = VexPlus(argv[1]);
  vex_file.parseVex();

  { // sfxc_control
    Json::Reader reader;
    std::ifstream in(argv[2]);
    bool ok = reader.parse(in, ctrl_file);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      std::cout  << "Failed to parse configuration\n"
                 << reader.getFormatedErrorMessages();
      return 1;
    }
  }

  assert(vex_file.N_Modes() == 1);

  set_stations();

  //create and fill a correlator control file
  std::vector< std::vector<std::string> > channels;
  channels = get_channels(vex_file, ctrl_file);
  for (size_t channel = 0; channel < channels.size(); channel++) {
    if (channels[channel].size() == 1) {
      make_ccf(channels[channel][0], "");
    } else {
      make_ccf(channels[channel][0], channels[channel][1]);
    }
  }
  return 0;
}

void make_ccf(std::string const &channel1, std::string const &channel2) {
  std::string scan_name = ctrl_file["scan"].asString();

  //open ccf, channel+1 since the channels are 1-based in the vex-file
  std::ofstream cc_file(generate_ccf_filename(ctrl_file["ccfdir"].asString(),
                                              vex_file.ExperName(),
                                              ctrl_file["scan"].asString(),
                                              channel1, channel2).c_str());
  assert(cc_file.is_open());


  string CorrelationJob = vex_file.ExperName()+"_"+scan_name+"_"+channel1;
  if (channel2.size() > 0) {
    CorrelationJob += "_" + channel2;
  }
  time_t creation_time;
  time(&creation_time);

  cc_file<<"CCF_REV " << REVISION <<"\n\n";
  cc_file<<"# Correlator control file for sfxc.\n";
  cc_file<<"# Generated on   : " << ctime(&creation_time) << "\n";

  cc_file<<"# This is a generated file. \n"; 
  cc_file<<"# 1) Blank lines and everything after # is regarded as comment.\n";
  cc_file<<"# 2) Replace indicated text including delimiters !* *!\n";
  cc_file<<"# 3) Change user changeable values if necessary.\n";
  cc_file<<"#    sfxc should run using the default values.\n";
  cc_file<<"# 4) Do not change other parameters.\n\n";
  

  //write run parameters block, default values
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"MESSAGELVL  " << ctrl_file["messagelevel"].asInt()
         << " #User changeable.\n";
  cc_file<<"INTERACTIVE 0  #User changeable.\n";
  cc_file<<"RUNOPTION   1  #User changeable.\n";
  {
    if (!ctrl_file["refstation"].isString()) {
      ctrl_file["refstation"] = "";
    }
    int ref_station_nr = -1;
    for (size_t i=0; i<stations.size(); i++) {
      if (vex_file.Station(stations[i]) == 
          ctrl_file["refstation"].asString()) {
        ref_station_nr = i;
      }
    }
    if (ref_station_nr >= 0) {
      cc_file <<"REFSTATION1  " << ref_station_nr << " #User changeable.\n";
      if (channel2.size() > 0) {
        cc_file <<"REFSTATION2  " << ref_station_nr+stations.size() 
                << " #User changeable.\n";
      } else {
        cc_file <<"REFSTATION2  -1 #User changeable.\n";
      }
    } else {
      cc_file <<"REFSTATION1  -1 #User changeable.\n";
      cc_file <<"REFSTATION2  -1 #User changeable.\n";
    }
  }
  cc_file<<"\n";

  //write general parameters block
  
  //name of the experiment nad correlation job
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"EXPERIMENT       " << vex_file.ExperName() << endl;
  cc_file<<"JOB              " << CorrelationJob << endl;

  //mode id and index  
  //cc_file<<"MODE_ID          " << vex_file.Mode(vex_file.Mode(0)) << "  " << vex_file.Mode(0) << endl;
  //channel id and index
  //cc_file<<"CHANNEL_ID       " << LFT << "  " << ChanChI << endl;
  //channel id and index, other polarisation
//   if (ChanChIop != -1)
//     cc_file<<"CHANNEL_ID       " << LFTop << "  " << ChanChIop << endl;
  //start time
  cc_file<< "START            " << ctrl_file["start"].asString()
         << " #User changeable.\n";
  //scan duration 
  cc_file<<"DURATION         " << ctrl_file["duration"] << " #User changeable.\n";
  //rndhdr
  cc_file<<"RNDHDR           1  #User changeable.\n";
  if (channel2.size() > 0) {
    cc_file<<"NSTATIONS        " << 2*stations.size() << endl;
  } else {
    cc_file<<"NSTATIONS        " << stations.size() << endl;
  }
  cc_file << std::endl;

  //directory for output
  cc_file<<"OUTDIR           " << ctrl_file["outdir"].asString() << std::endl;;
  //logfile name
  cc_file<<"LOGFILE          " << CorrelationJob << ".log #User changeable.\n";
  //correlation file name
  cc_file<<"CORFILE          " 
         << generate_cor_filename("",
                                  vex_file.ExperName(),
                                  ctrl_file["scan"].asString(),
                                  channel1, channel2).c_str()
         << "\n";
  cc_file<<"\n";

  int ch1_int = -1, ch2_int = -1;
  for (size_t ch=0; 
       ch<vex_file.N_FreqChans(vex_file.Station(0), vex_file.Mode(0)); 
       ch++) {
    if (channel1 == vex_file.Link_freq_track(vex_file.Station(0), 
                                             vex_file.Mode(0),
                                             ch)) {
      ch1_int = ch;
    }
    if (channel2 == vex_file.Link_freq_track(vex_file.Station(0), 
                                             vex_file.Mode(0),
                                             ch)) {
      ch2_int = ch;
    }
  }
  assert(ch1_int != -1);
  assert((channel2.size() == 0) || (ch2_int != -1));


  //bandwidth input
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"SKYFREQ     "
         << vex_file.SkyFreq(vex_file.Station(0), vex_file.Mode(0), ch1_int) * 1000000.
         << endl;
  cc_file<<"BWIN        "
         << vex_file.BW(vex_file.Station(0),vex_file.Mode(0),ch1_int) \
         << std::endl;
  //side band U or L
  cc_file<< "SIDEBAND    "
         << vex_file.SideBand( vex_file.Station(0), vex_file.Mode(0),ch1_int)
         << endl;
  //fft length in delay correction
  cc_file<<"N2FFTDEL     "
         << ctrl_file["number_of_lags"].asInt() << std::endl;
  //delay columns
  cc_file<<"DELCOLS     1 1 1  #User changeable.\n";
  cc_file<<"\n";

  //filter switch
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"FILTER      0  #User changeable.\n";
  //filter bandwidth
  cc_file<<"BWFL        0  #Set if FILTER=1\n";
  //filter start frequency
  cc_file<<"STARTF      0  #Set if FILTER=1\n";
  //filter resolution
  cc_file<<"DELTAF      0  #Set if FILTER=1\n";
  //filter oversampling
  cc_file<<"OVRFL       1  #Set if FILTER=1\n";
  cc_file<<"\n";

  //correlation fft length
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"N2FFTCORR   " << ctrl_file["number_of_lags"].asInt() << std::endl;
  //correlation overlap
  cc_file<<"OVRLP       0.0  #User changeable.\n";
  //correlation time to average
  cc_file<<"TIME2AVG    " << ctrl_file["integration_time"].asDouble() << "\n";
  //correlation padding
  cc_file<<"PAD         2    #User changeable.\n";
  cc_file<<"\n\n";

  //write block for each station
  for (size_t station=0; station<stations.size(); station++) {
    write_station_block(cc_file, station, station, 
                        ch1_int, ch1_int, scan_name);
  }
  if (channel2.size() > 0) {
    for (size_t station=0; station<stations.size(); station++) {
      write_station_block(cc_file, 
                          station+stations.size(), 
                          station, 
                          ch2_int, ch1_int, scan_name);
    }
  }

  //close ccf
  cc_file.close();
}


//write block for each station
void write_station_block(ofstream &cc_file, 
                         int nth_station_in_ccf, 
                         int station, 
                         int channel, int channel_for_delay_table,
                         std::string const &scan_name) {
  string StationStr(vex_file.Station(stations[station]));
  int fo=vex_file.FanOut(StationStr, vex_file.Mode(0));
  //station_name data_type
  cc_file<<"#_____________________________________________________________\n";
  cc_file << "ST" << setfill('0') << setw(4) << nth_station_in_ccf << "  " 
          << StationStr //<< (nth_station_in_ccf == station? "-L" : "-R") 
          << " " 
          << vex_file.TrackFormat(StationStr,vex_file.Mode(0)) << endl << endl;
  //fan_out
  cc_file<<"FO          " << fo << endl;
  //bits_per_sample
  int BPS=vex_file.N_Bits(StationStr,vex_file.Mode(0));
  cc_file<<"BPS         " << BPS << endl;
  //nr_of_headstacks
  int NHS=1;
  for (int j=0; j<vex_file.N_TrackLines(StationStr,vex_file.Mode(0)); j++)
    if (vex_file.HeadstackNr(StationStr,vex_file.Mode(0),j)>1) 
      NHS=vex_file.HeadstackNr(StationStr,vex_file.Mode(0),j);
  cc_file<<"NHS         " << NHS << endl;
  //byte offset
  cc_file<<"BOFF        0  #User changeable.\n";
  //synchronisation track for headstack 1
  cc_file<<"SYNHS1      1  #User changeable.\n";
  //synchronisation track for headstack 2
  cc_file<<"SYNHS2      2  #User changeable.\n";
  //mk4file
  cc_file<<"MK4FILE     " << ctrl_file["mk4dir"].asString() <<"/" 
         << ctrl_file["mk4file"][stations[station]].asString()
         << std::endl;

  int trnr=0;
  string RTF,SM;//resolved track frequency, sign or mag string
  //sign
  string LFT=vex_file.Link_freq_track(vex_file.Station(0),
                                      vex_file.Mode(0),
                                      channel);
  while (trnr < vex_file.N_TrackLines(StationStr, vex_file.Mode(0))) {
    RTF=vex_file.Resolve_track_freq(StationStr, vex_file.Mode(0),trnr);
    SM=vex_file.TrackSignMag(StationStr, vex_file.Mode(0),trnr);
    if (RTF == LFT && SM=="sign"){
      cc_file<<"SIGN        " << 
        vex_file.HeadstackNr(StationStr,vex_file.Mode(0),trnr) << " ";
      for (int j=0; j<fo; j++)
        cc_file << vex_file.TrackNr(StationStr,vex_file.Mode(0),trnr*fo+j) << " ";
      cc_file << endl;
      break;
    }
    trnr++;
  }

  //magn
  trnr=0;
  if(BPS==2) {
    while (trnr < vex_file.N_TrackLines(StationStr, vex_file.Mode(0))) {
      RTF=vex_file.Resolve_track_freq(StationStr, vex_file.Mode(0),trnr);
      SM=vex_file.TrackSignMag(StationStr, vex_file.Mode(0),trnr);
      if (RTF == LFT && SM=="mag"){
        cc_file<<"MAGN        " << 
          vex_file.HeadstackNr(StationStr,vex_file.Mode(0),trnr) << " ";
        for (int j=0; j<fo; j++)
          cc_file << vex_file.TrackNr(StationStr,vex_file.Mode(0),trnr*fo+j) << " ";
        cc_file << endl;
        break;
      }
      trnr++;
    }
  }

  //MK4END
  cc_file<<"MK4END\n\n";
    
  //delay_table
  string deltbl =
    generate_del_filename(ctrl_file["deldir"].asString(),
                          vex_file.ExperName(),
                          ctrl_file["scan"].asString(),
                          vex_file.Link_freq_track(vex_file.Station(0), 
                                                   vex_file.Mode(0),
                                                   channel_for_delay_table),
                          vex_file.Station(stations[station]));
  cc_file<<"DELAYTABLE  " << deltbl << "\n";
  //DELAYEND
  cc_file<<"DELAYEND\n\n\n";
}  


void set_stations() {
  stations.clear(); 

  std::set<std::string> ctrl_stations;
  if (ctrl_file["stations"].isArray()) {
    for (int i=0; i<ctrl_file["stations"].size(); i++) {
      ctrl_stations.insert(ctrl_file["stations"][i].asString());
     }
  }

  for (int i=0; i < vex_file.N_Stations(); i++) {
    if (ctrl_stations.find(vex_file.Station(i)) != ctrl_stations.end()) {
      stations.push_back(i);
    }
  }

  // Print error message if necessary
  if (ctrl_stations.size() != stations.size()) {
    std::cout << "Could not find all stations: " << std::endl
              << "requested: ";
    for (int i=0; i<ctrl_file["stations"][i].size(); i++) {
      std::cout << ctrl_file["stations"][i].asString() << ",";
    }
    std::cout << std::endl;
    std::cout << "found: ";
    for (size_t i=0; i<stations.size(); i++) {
      std::cout << stations[i] << ",";
    }
    std::cout << std::endl;
  }
}
