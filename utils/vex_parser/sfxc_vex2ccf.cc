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
void make_ccf(int channel1, int channel2);
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
  int nChannels = vex_file.N_FreqChans(vex_file.Station(0),
                                       vex_file.Mode(0));
  for (int channel = 0; channel < nChannels; channel ++) {
    if ((channel<nChannels-1) &&
        (vex_file.SkyFreq(vex_file.Station(0),vex_file.Mode(0),channel) ==
         vex_file.SkyFreq(vex_file.Station(0),vex_file.Mode(0),channel+1))) {
      make_ccf(channel, channel+1);
      channel++;
    } else {
      // Next channel is not the other polarisation
      make_ccf(channel, -1);
    }
  }
  return 0;
}

void make_ccf(int channel1, int channel2) {
  std::string scan_name = ctrl_file["scan"].asString();

  //open ccf, channel+1 since the channels are 1-based in the vex-file
  std::ofstream cc_file(generate_ccf_filename(ctrl_file["ccfdir"].asString(),
                                              vex_file.ExperName(),
                                              ctrl_file["scan"].asString(),
                                              channel1+1, channel2+1).c_str());
  assert(cc_file.is_open());


  string CorrelationJob = 
    vex_file.ExperName()+"_"+
    scan_name+"_"+
    vex_file.Link_freq_track(vex_file.Station(0),vex_file.Mode(0),channel1);
  if (channel2 >= 0) {
    CorrelationJob +=
      "_" + 
      (vex_file.Link_freq_track(vex_file.Station(0),vex_file.Mode(0),channel2));
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
      if (channel2 >= 0) {
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
  if (channel2 >= 0) {
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
                                  channel1+1, channel2+1).c_str()
         << "\n";
  cc_file<<"\n";


  //bandwidth input
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"SKYFREQ     "<< 
    vex_file.SkyFreq(vex_file.Station(0), vex_file.Mode(0), channel1) * 1000000. << endl;
  cc_file<<"BWIN        "<< vex_file.BW(vex_file.Station(0),vex_file.Mode(0),channel1) <<endl;
  //side band U or L
  cc_file<<"SIDEBAND    "<< vex_file.SideBand( vex_file.Station(0), vex_file.Mode(0),channel1)<<endl;
  //fft length in delay correction
  cc_file<<"N2FFTDEL     "<< ctrl_file["number_of_lags"].asInt() << std::endl;
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
                        channel1, channel1, scan_name);
  }
  if (channel2 >= 0) {
    for (size_t station=0; station<stations.size(); station++) {
      write_station_block(cc_file, 
                          station+stations.size(), 
                          station, 
                          channel2, channel1, scan_name);
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
    vex_file.ExperName()+"_"+scan_name+"_"+
    vex_file.Station(stations[station])+"_"+
    vex_file.Link_freq_track(vex_file.Station(0),
                             vex_file.Mode(0),
                             channel_for_delay_table)+".del";
  cc_file<<"DELAYTABLE  " 
         << generate_del_filename(ctrl_file["deldir"].asString(),
                                  vex_file.ExperName(),
                                  ctrl_file["scan"].asString(),
                                  channel_for_delay_table+1,
                                  vex_file.Station(stations[station]))
         << "\n";
  //DELAYEND
  cc_file<<"DELAYEND\n\n\n";
}  


  
// //create dcf and write proper values (dcf=delay modelcontrol file)
// int create_dcf(string vexStr, VexPlus &vex_file, int vex_file.Mode(0), int *stations, int ScanChI, 
//    int stations.size(), int ChanChI, int ChanChIop)
// {

//   string mode=vex_file.Mode(vex_file.Mode(0));
//   string LFT=vex_file.Link_freq_track(vex_file.Station(stations[0]),mode,ChanChI);
//   string CorrelationJob = vex_file.ExperName()+"_"+vex_file.ScanName(ScanChI)+"_"+LFT;
//   string LFTop="";
//   if (ChanChIop != -1){
//     LFTop=vex_file.Link_freq_track(vex_file.Station(stations[0]),mode,ChanChIop);
//     CorrelationJob = vex_file.ExperName()+"_"+vex_file.ScanName(ScanChI)+"_"+LFT+"_"+LFTop;
//   }

//   if (stations.size() < vex_file.N_stations()) {
//     CorrelationJob = CorrelationJob + "_";
//     for (int i=0; i<stations.size(); i++)
//       CorrelationJob = CorrelationJob + vex_file.Station(stations[i]);
//   }

//   string dcf_name;    
//   dcf_name=CorrelationJob+".dcf";
//   std::cerr << dcf_name << std::endl;
  
//   //open ccf
//   ofstream dc_file(dcf_name.c_str());

//   time_t creation_time;
//   time(&creation_time);
//   dc_file<<"#_____________________________________________________________\n";
//   dc_file<<"# DCF_REV " << REVISION <<"\n";
//   dc_file<<"# Control file for delmo.\n";
//   dc_file<<"# Using data from: " << vexStr << "\n";
//   dc_file<<"# Generated on: " <<  ctime(&creation_time) << "\n\n";  

//   //name of the experiment + source + scanname + channel
//   dc_file<<"#EXPERIMENT___________________________________________________\n";
//   dc_file<<"experiment      " << vex_file.ExperName() << endl;
//   //mode id and index
//   dc_file<<"Mode ID         " << mode << endl;
//   //scan id
//   dc_file<<"Scan ID         " << vex_file.ScanName(ScanChI) << endl;
//   //channel id and index
//   dc_file<<"Channel ID      " << LFT << endl;
//   dc_file<< endl;


//   int SourceChI;
//   dc_file<<"#SOURCE_______________________________________________________\n";
//   for(int j= 0; j<vex_file.N_Sources(); j++)
//     if(vex_file.SourceName(j) == vex_file.ScanSource(vex_file.Station(stations[0]),ScanChI))
//       SourceChI=j;
//   dc_file<<"source_name     " <<  vex_file.SourceName(SourceChI)<<endl;
//   dc_file<<"source_ra       " <<  setprecision(12) << vex_file.Source_RA(SourceChI)<<endl;
//   dc_file<<"source_dec      " <<  vex_file.Source_Dec(SourceChI)<<endl;
//   dc_file<<endl;

//   {
//     dc_file<<"#EOP__________________________________________________________\n";
//     dc_file<<"tai_utc         " << vex_file.TAI_UTC()<<endl;
//     string eop_ref_epoch=vex_file.EOPEpoch();
//     int year = str_to_long(eop_ref_epoch,0,4);  //pos=0, length=4
//     int doy = str_to_long(eop_ref_epoch,5,3);
//     int month, day;
//     yd2md(year,doy,month,day);
//     dc_file<<"eop_ref_epoch   " << JD(year,month,day) <<" julian day"<<endl;
//     dc_file<<"num_eop_points  " << vex_file.N_EOP_Points()<<endl;
//     for(int i=0; i<vex_file.N_EOP_Points(); i++) 
//       dc_file<< "ut1_utc[" << i << "]      " << vex_file.UT1_UTC(i) << endl;
//     for(int i=0; i<vex_file.N_EOP_Points(); i++) 
//       dc_file<< "x_wobble[" << i << "]     " << vex_file.XWobble(i) << endl;
//     for(int i=0; i<vex_file.N_EOP_Points(); i++) 
//       dc_file<< "y_wobble[" << i << "]     " << vex_file.YWobble(i) << endl;
//     dc_file<<endl;
//   }

//   int year_start;
//   {
//     dc_file<<"#SCAN_________________________________________________________\n";
//     string startTime=vex_file.ScanStart(vex_file.Station(stations[0]), ScanChI);
//     year_start = str_to_long(startTime,0,4);  //pos=0, length=4
//     int doy = str_to_long(startTime,5,3);
//     int month, day;
//     int hr  = str_to_long(startTime,9,2);
//     int min = str_to_long(startTime,12,2);
//     int sec = str_to_long(startTime,15,2);
//     yd2md(year_start,doy,month,day);    
//     dc_file<<"year            " << year_start << endl;  //pos=0, length=4
//     dc_file<<"month           " << month << endl;
//     dc_file<<"day             " << day << endl;
//     dc_file<<"hr              " << hr  << endl;
//     dc_file<<"min             " << min << endl;
//     dc_file<<"sec             " << sec << endl;
//     dc_file<<"scan_duration   " << 
//       vex_file.ScanDuration(vex_file.Station(stations[0]), ScanChI)  << endl;
//     long scan_start = sec + min*60 + hr*3600 + doy*86400;
//     dc_file<<"scan_start      " << scan_start << " (sec) " << startTime << endl;
//     dc_file<<endl;
//   }


//   dc_file<<"#RFREQ________________________________________________________\n";
//   dc_file<<"freq            " << 
//     vex_file.SkyFreq(vex_file.Station(stations[0]), mode, ChanChI) * 1000000. << endl;
//   dc_file<<endl;

//   dc_file<<"#Station_data_________________________________________________\n";
//   //nr of stations
//   dc_file<<"nr_of_stations  " << stations.size() << endl << endl;

//   //for each station write relevant parameters to dcf
//   int axis_type=0;
//   for (int i=0; i<stations.size(); i++)
//   {
//     string deltbl =
//       vex_file.ExperName()+"_"+vex_file.ScanName(ScanChI)+"_"+
//       vex_file.Station(stations[i])+"_"+LFT+".del";

//     dc_file<<"#SITE_________________________________________________________\n";
//     dc_file<<"DELAYTABLE            "<< deltbl << endl;
//     dc_file<<"site_name             "<< vex_file.Site(vex_file.Station(stations[i]))<<endl;
//     dc_file<<"site_position_x       "<< setprecision(12) << vex_file.SiteX(vex_file.Station(stations[i]))<< endl;
//     dc_file<<"site_position_y       "<< vex_file.SiteY(vex_file.Station(stations[i]))<< endl;
//     dc_file<<"site_position_z       "<< vex_file.SiteZ(vex_file.Station(stations[i]))<< endl;
//     if (vex_file.AxisMount(vex_file.Station(stations[i])) == "az_el") axis_type=3;
//     if (vex_file.AxisMount(vex_file.Station(stations[i])) == "ha_dec") axis_type=4;
//     dc_file<<"axis_type             "<< axis_type << endl;
//     dc_file<<"axis_offset           "<< vex_file.AxisOffset(vex_file.Station(stations[i]))<<endl;
//     dc_file<<"clock_early           "<< vex_file.ClockOffset(vex_file.Station(stations[i]))<< " (usec)\n";
//     dc_file<<"clock_rate            "<< vex_file.ClockRate(vex_file.Station(stations[i]))<< " (usec/sec)\n";
//     string clock_epoch = vex_file.ClockEpoch(vex_file.Station(stations[i]));
//     dc_file<<"clock_epoch           "<< ceps(clock_epoch,year_start) <<" (sec) " << clock_epoch<<endl;
//     dc_file<<endl;
//   }

//   //close ccf
//   dc_file.close();

// }




// long str_to_long (std::string inString, int pos, int length)
// {
//   std::string str=inString.substr(pos,length);
//   char tmp[length+1];
//   strcpy(tmp,str.c_str());
  
//   char *endp;
//   long sval = strtol(tmp, &endp, 10);
//   if (endp == tmp) {
//     fprintf(stderr,"**** Unable to convert string %s into long\n",tmp);
//     return -1;
//   } else {
//     return sval;
//   }  
// }


// //this function is called by yd2md
// //input month, K
// //output month length
// int MonthLength(int M, int K)
// {
//  return M==2 ? 30-K : (275*(M+1)/9) - (275*M/9);
// }

// //input  year, day of year
// //output month, day of month
// void yd2md(int year, int doy, int &month, int &dom){

//   int K = 2 - (year%4==0); // 2 - Leap, 1901-2099

//   int rest_of_days=doy;
//   int current_month=1;
//   int length_current_month=MonthLength(current_month,K);
//   while(rest_of_days > length_current_month) {
//     rest_of_days -= length_current_month;
//     current_month++;
//     length_current_month=MonthLength(current_month,K);    
//   }
//   dom = rest_of_days;
//   month = current_month;
// }

// //input year month day
// //output Julian Day
// long long JD(int y, int m, int d)
// {
//    return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
//           ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
//           ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
//           d - 32075;  
// }


// // Given: some year, pY
// // Returns 366 if pY is a leap year, 365 if it is not
// int DaysPerYear(int pY)
// {
//   int result=365;
//   if (pY % 400 ==  0)  result = 366;
//   else if (pY % 100 == 0) result = 365;
//   else if (pY % 4 == 0) result = 366;
//   else result = 365;
//   return result;
// }


// //returns clock epoch in seconds for timeString
// //assumption: |year-ref_year|<=1
// long ceps(string timeString, int ref_year)
// {
//   long clock_epoch=0;
//   int year = str_to_long(timeString,0,4);  //pos=0, length=4
//   int doy = str_to_long(timeString,5,3);
//   int hr = str_to_long(timeString,9,2);
//   int min = str_to_long(timeString,12,2);
//   int sec = str_to_long(timeString,15,2);
//   assert(abs(year-ref_year)<=1);
//   if (ref_year == year) {
//     clock_epoch = sec + 60*min + 3600*hr + 86400*doy;
//   } else {
//     int DpY = DaysPerYear(ref_year);
//     clock_epoch = sec + 60*min + 3600*hr + 86400*doy + (year-ref_year)*DpY * 86400;
//   }
//   return clock_epoch;
// }


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
