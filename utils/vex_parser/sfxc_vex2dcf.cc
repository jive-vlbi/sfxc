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
int scan_nr = -1;
              
void make_dcf(int channel1, int channel2);
long str_to_long (std::string inString, int pos, int length);

//this function is called by yd2md
//input month, K
//output month length
int MonthLength(int M, int K);

//input  year, day of year
//output month, day of month
void yd2md(int year, int doy, int &month, int &dom);

//input year month day
//output Julian Day
long long JD(int y, int m, int d);

// Given: some year, pY
// Returns 366 if pY is a leap year, 365 if it is not
int DaysPerYear(int pY);

//returns clock epoch in seconds for timeString
//assumption: |year-ref_year|<=1
long ceps(string timeString, int ref_year);

//MAIN
int main (int argc, char *argv[])
{

  if (argc != 3){
    cout << "purposes:a) create ccf for sfxc by extracting relevant\n";
    cout << "            vex parameters and setting defaults values\n";
    cout << "            for the others.\n";
    cout << "         b) create dcf and a local copy of DE405_le.jpl for \n";
    cout << "            application delmo.\n\n";
    cout << "usage  : sfxc_vex2dcf <vexfilename> <sfxc-ctrl>\n";
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

  for (size_t scan = 0; scan < vex_file.N_Scans(vex_file.Station(0)); scan++) {
    if (ctrl_file["scan"].asString() == vex_file.ScanName(scan)) {
      scan_nr = scan;
      break;
    }
  }
  assert(scan_nr >= 0);

  int nChannels = vex_file.N_FreqChans(vex_file.Station(0),
                                       vex_file.Mode(0));
  for (int channel = 0; channel < nChannels; channel ++) {
    assert(ctrl_file["stations"].isArray());
    for (size_t station=0; station<ctrl_file["stations"].size(); station++) {
      make_dcf(channel, station);
    }
  }

  return 0;
}

void make_dcf(int channel, int station) {
  string LFT=vex_file.Link_freq_track(vex_file.Station(station),vex_file.Mode(0),channel);
  string CorrelationJob = vex_file.ExperName()+"_"+ctrl_file["scan"].asString()+"_"+LFT;

  string dcf_name=generate_dcf_filename(ctrl_file["deldir"].asString(),
                                        vex_file.ExperName(),
                                        ctrl_file["scan"].asString(),
                                        channel+1);


  //open dcf
  ofstream dc_file(dcf_name.c_str());

  time_t creation_time;
  time(&creation_time);
  dc_file<<"#_____________________________________________________________\n";
  dc_file<<"# DCF_REV " << REVISION <<"\n";
  dc_file<<"# Control file for delmo.\n";
  dc_file<<"# Generated on: " <<  ctime(&creation_time) << "\n\n";  

  //name of the experiment + source + scanname + channel
  dc_file<<"#EXPERIMENT___________________________________________________\n";
  dc_file<<"experiment      " << vex_file.ExperName() << endl;
  //channel id and index
  dc_file<<"Channel ID      " << LFT << endl;
  dc_file<< endl;


  int SourceChI;
  dc_file<<"#SOURCE_______________________________________________________\n";
  for(int j= 0; j<vex_file.N_Sources(); j++) {
    assert(scan_nr >= 0);
    if(vex_file.SourceName(j) == 
       vex_file.ScanSource(vex_file.Station(station),scan_nr)) {
      SourceChI=j;
    }
  }
  dc_file<<"source_name     " <<  vex_file.SourceName(SourceChI)<<endl;
  dc_file<<"source_ra       " <<  setprecision(12) << vex_file.Source_RA(SourceChI)<<endl;
  dc_file<<"source_dec      " <<  vex_file.Source_Dec(SourceChI)<<endl;
  dc_file<<endl;

  {
    dc_file<<"#EOP__________________________________________________________\n";
    dc_file<<"tai_utc         " << vex_file.TAI_UTC()<<endl;
    string eop_ref_epoch=vex_file.EOPEpoch();
    int year = str_to_long(eop_ref_epoch,0,4);  //pos=0, length=4
    int doy = str_to_long(eop_ref_epoch,5,3);
    int month, day;
    yd2md(year,doy,month,day);
    dc_file<<"eop_ref_epoch   " << JD(year,month,day) <<" julian day"<<endl;
    dc_file<<"num_eop_points  " << vex_file.N_EOP_Points()<<endl;
    for(int i=0; i<vex_file.N_EOP_Points(); i++) 
      dc_file<< "ut1_utc[" << i << "]      " << vex_file.UT1_UTC(i) << endl;
    for(int i=0; i<vex_file.N_EOP_Points(); i++) 
      dc_file<< "x_wobble[" << i << "]     " << vex_file.XWobble(i) << endl;
    for(int i=0; i<vex_file.N_EOP_Points(); i++) 
      dc_file<< "y_wobble[" << i << "]     " << vex_file.YWobble(i) << endl;
    dc_file<<endl;
  }

  int year_start;
  {
    dc_file<<"#SCAN_________________________________________________________\n";
    string startTime=vex_file.ScanStart(vex_file.Station(station), scan_nr);
    year_start = str_to_long(startTime,0,4);  //pos=0, length=4
    int doy = str_to_long(startTime,5,3);
    int month, day;
    int hr  = str_to_long(startTime,9,2);
    int min = str_to_long(startTime,12,2);
    int sec = str_to_long(startTime,15,2);
    yd2md(year_start,doy,month,day);    
    dc_file<<"year            " << year_start << endl;  //pos=0, length=4
    dc_file<<"month           " << month << endl;
    dc_file<<"day             " << day << endl;
    dc_file<<"hr              " << hr  << endl;
    dc_file<<"min             " << min << endl;
    dc_file<<"sec             " << sec << endl;
    dc_file<<"scan_duration   " << 
      vex_file.ScanDuration(vex_file.Station(station), scan_nr)  << endl;
    long scan_start = sec + min*60 + hr*3600 + doy*86400;
    dc_file<<"scan_start      " << scan_start << " (sec) " << startTime << endl;
    dc_file<<endl;
  }


  dc_file<<"#RFREQ________________________________________________________\n";
  dc_file<<"freq            " << 
    vex_file.SkyFreq(vex_file.Station(station), vex_file.Mode(0), channel) * 1000000. << endl;
  dc_file<<endl;

  dc_file<<"#Station_data_________________________________________________\n";
  //nr of stations
  dc_file<<"nr_of_stations  " << vex_file.N_Stations() << endl << endl;

  //for each station write relevant parameters to dcf
  int axis_type=0;
  for (int station=0; station<vex_file.N_Stations(); station++)
  {


    dc_file<<"#SITE_________________________________________________________\n";
    string deltbl = generate_del_filename(ctrl_file["deldir"].asString(),
                                          vex_file.ExperName(),
                                          ctrl_file["scan"].asString(),
                                          channel+1,
                                          vex_file.Station(station));
    dc_file<<"DELAYTABLE            "<< deltbl << endl;
    dc_file<<"site_name             "<< vex_file.Site(vex_file.Station(station))<<endl;
    dc_file<<"site_position_x       "<< setprecision(12) << vex_file.SiteX(vex_file.Station(station))<< endl;
    dc_file<<"site_position_y       "<< vex_file.SiteY(vex_file.Station(station))<< endl;
    dc_file<<"site_position_z       "<< vex_file.SiteZ(vex_file.Station(station))<< endl;
    if (vex_file.AxisMount(vex_file.Station(station)) == "az_el") axis_type=3;
    if (vex_file.AxisMount(vex_file.Station(station)) == "ha_dec") axis_type=4;
    dc_file<<"axis_type             "<< axis_type << endl;
    dc_file<<"axis_offset           "<< vex_file.AxisOffset(vex_file.Station(station))<<endl;
    dc_file<<"clock_early           "<< vex_file.ClockOffset(vex_file.Station(station))<< " (usec)\n";
    dc_file<<"clock_rate            "<< vex_file.ClockRate(vex_file.Station(station))<< " (usec/sec)\n";
    string clock_epoch = vex_file.ClockEpoch(vex_file.Station(station));
    dc_file<<"clock_epoch           "<< ceps(clock_epoch,year_start) <<" (sec) " << clock_epoch<<endl;
    dc_file<<endl;
  }

  //close ccf
  dc_file.close();
}

long str_to_long (std::string inString, int pos, int length)
{
  std::string str=inString.substr(pos,length);
  char tmp[length+1];
  strcpy(tmp,str.c_str());
  
  char *endp;
  long sval = strtol(tmp, &endp, 10);
  if (endp == tmp) {
    fprintf(stderr,"**** Unable to convert string %s into long\n",tmp);
    return -1;
  } else {
    return sval;
  }  
}


//this function is called by yd2md
//input month, K
//output month length
int MonthLength(int M, int K)
{
 return M==2 ? 30-K : (275*(M+1)/9) - (275*M/9);
}

//input  year, day of year
//output month, day of month
void yd2md(int year, int doy, int &month, int &dom){

  int K = 2 - (year%4==0); // 2 - Leap, 1901-2099

  int rest_of_days=doy;
  int current_month=1;
  int length_current_month=MonthLength(current_month,K);
  while(rest_of_days > length_current_month) {
    rest_of_days -= length_current_month;
    current_month++;
    length_current_month=MonthLength(current_month,K);    
  }
  dom = rest_of_days;
  month = current_month;
}

//input year month day
//output Julian Day
long long JD(int y, int m, int d)
{
   return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
          ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
          ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
          d - 32075;  
}


// Given: some year, pY
// Returns 366 if pY is a leap year, 365 if it is not
int DaysPerYear(int pY)
{
  int result=365;
  if (pY % 400 ==  0)  result = 366;
  else if (pY % 100 == 0) result = 365;
  else if (pY % 4 == 0) result = 366;
  else result = 365;
  return result;
}


//returns clock epoch in seconds for timeString
//assumption: |year-ref_year|<=1
long ceps(string timeString, int ref_year)
{
  long clock_epoch=0;
  int year = str_to_long(timeString,0,4);  //pos=0, length=4
  int doy = str_to_long(timeString,5,3);
  int hr = str_to_long(timeString,9,2);
  int min = str_to_long(timeString,12,2);
  int sec = str_to_long(timeString,15,2);
  assert(abs(year-ref_year)<=1);
  if (ref_year == year) {
    clock_epoch = sec + 60*min + 3600*hr + 86400*doy;
  } else {
    int DpY = DaysPerYear(ref_year);
    clock_epoch = sec + 60*min + 3600*hr + 86400*doy + (year-ref_year)*DpY * 86400;
  }
  return clock_epoch;
}


