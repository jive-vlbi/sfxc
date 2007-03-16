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

//c++ includes
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "vexplus.h"

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


//month length
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

long long JD(int y, int m, int d)
{
   return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
          ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
          ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
          d - 32075;  
}

int main (int argc, char *argv[])
{

  if (argc != 2){
    cout << "purposes:a) create ccf for sfxc by extracting relevant\n";
    cout << "         vex parameters and setting defaults values\n";
    cout << "         for the others.\n";
    cout << "         b) create dcf for delmo.\n";
    cout << "usage  : vex2ccf vexfilename <ret>\n";
    return 0;
  }

  const string vexStr(argv[1]);

  //---------------------------------------------------------------------------  
  cout << "\n\nvex to ccf conversion for " << vexStr << ".\n" << endl;

  //initialize
  VexPlus VP(vexStr);

  //open vex file
  VP.parseVex();

  //$GLOBAL
  cout << "\n$GLOBAL\n";
  cout << "ExperName  " << VP.ExperName() << endl;
  
  //$MODE
  cout << "\n$MODE\n";
  int n_modes = VP.N_Modes();
  cout << "N_Modes    " << n_modes << endl;
  cout << "Modes  ";
  for (int i=0; i<n_modes; i++)
    cout << i << ":" << VP.Mode(i) << " ";
  cout << endl;  
    
  string ModeCh(VP.Mode(0));
  int ModeChI;
  if (n_modes > 1) {
    while (1) {
      cout << "Enter mode number of your choice\n";
      cin >> ModeChI;
      if (0 <= ModeChI && ModeChI < n_modes) break;
      else cout << "Illegal value!\n";
    } 
    ModeCh=VP.Mode(ModeChI);
  }  

  //$STATION
  cout << "\n$STATION\n";
  int n_stations = VP.N_Stations();
  cout << "N_Stations " << n_stations << endl;
  cout << "Stations   ";
  for (int i=0; i < n_stations; i++)
    cout << i << ":" << VP.Station(i) << " ";
  cout << endl << endl;
  
  int StatChI[32];
  int n_stat_ccf;//nr of stations for ccf
  if (n_stations > 1) {
    while (1) {
      cout << "Enter number of stations to correlate (0=select all)\n";
      cin >> n_stat_ccf;
      if ((2 <= n_stat_ccf && n_stat_ccf <= n_stations) || n_stat_ccf==0 ) break;
      else cout << "Illegal value!\n";
    }
    if (n_stat_ccf==0) {
      n_stat_ccf=n_stations;
      for (int i=0; i<n_stat_ccf; i++) { 
        StatChI[i] = i;
      }  
    } else {
      for (int i=0; i<n_stat_ccf; i++) {
        while (1) {
          cout << "Enter station number of your choice\n";
          cin >> StatChI[i];
          if (0 <= StatChI[i] && StatChI[i] < n_stations) break;
          else cout << "Illegal value!\n";
        }
      }
    }  
  }

  //$FREQ
  cout << "\n$FREQ\n";
  cout << "ChNr  ChId  Sky_freqs     BW\n";
  //asumption: all stations observe same frequencies
  int n_chans = VP.N_FreqChans(VP.Station(StatChI[0]),ModeCh);
  for (int i=0; i<n_chans; i++)
    cout << 
    setw(4)  << i <<
    setw(6)  << VP.Link_freq_track(VP.Station(StatChI[0]),ModeCh,i) <<
    setw(11) << VP.SkyFreq(VP.Station(StatChI[0]),ModeCh,i) <<
    setw(7)  << VP.BW(VP.Station(StatChI[0]),ModeCh,i)<< endl;
  cout << endl;
  int ChanChI; //Channel choice integer
  while (1) {
    cout << "Enter channel number for correlation:\n";
    cin >> ChanChI;
    if (0 <= ChanChI && ChanChI < n_chans) break;
    else cout << "Illegal value!\n";
  }  
  string LFT=VP.Link_freq_track(VP.Station(StatChI[0]),ModeCh,ChanChI);
  
  //$SCHED
  cout << "\n$SCHED\n";
  //asumption: all stations observe same source at the same time
  int n_scans = VP.N_Scans(VP.Station(StatChI[0]));
  cout << "N_Scans      " << n_scans << endl;
  cout << "ScNr         ScanId               Start         Source Duration\n";
  for (int i=0; i<n_scans; i++)
    cout << 
    setw(4)  << i <<
    setw(15) << VP.ScanName(i) <<
    setw(20) << VP.ScanStart(VP.Station(StatChI[0]),i) <<
    setw(15) << VP.ScanSource(VP.Station(StatChI[0]),i) <<
    setw(9)  << VP.ScanDuration(VP.Station(StatChI[0]),i) << endl;
  int ScanChI;
  while (1) {
    cout << "\nEnter scan number for correlation:\n";
    cin >> ScanChI;
    if (0 <= ScanChI && ScanChI < n_scans) break;
    else cout << "Illegal value!\n";
  }

  //---------------------------------------------------------------------------
  //extraction of vex data and write default data for other parameters

  //set ccf name    
  string Experiment = VP.ExperName()+"_"+
    VP.ScanSource(VP.Station(StatChI[0]),ScanChI)+"_"+
    VP.ScanName(ScanChI)+"_"+LFT;
  if (n_stat_ccf < n_stations) {
    Experiment = Experiment + "_";
    for (int i=0; i<n_stat_ccf; i++)
      Experiment = Experiment + VP.Station(StatChI[i]);
  }
  string ccf_name;
    
  cout << "Enter ccf name or enter A to generate name automatically. \n";
  cin >> ccf_name;
  if (ccf_name=="A") ccf_name=Experiment+".ccf";
  
  //open ccf
  ofstream cc_file(ccf_name.c_str());

  time_t creation_time;
  time(&creation_time);
  cc_file<<"CCF_REV " << REVISION <<"\n\n";
  cc_file<<"# Correlator control file for sfxc.\n";
  cc_file<<"# Using data from: " << vexStr << "\n";
  cc_file<<"# Generated on: " <<  ctime(&creation_time) << "\n";

  cc_file<<"# This is a generated file. \n"; 
  cc_file<<"# 1) Replace indicated text including delimiters !* *!\n";
  cc_file<<"# 2) Change Default values if necessary and \n";
  cc_file<<"#    remove text between !* *! including these delimiters\n";
  cc_file<<"#    sfxc should run using default values.\n";
  cc_file<<"# 3) Only change parameters marked by !* *!\n";
  cc_file<<"#    Leave other lines unchanged!\n";
  cc_file<<"# 4) Comment lines can be added as long as they \n";
  cc_file<<"#    do not start with one of the CAPITAL keywords.\n\n";
  

  //write run parameters block, default values
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"MESSAGELVL  1  !* Default Value *!\n";
  cc_file<<"INTERACTIVE 0  !* Default Value *!\n";
  cc_file<<"RUNOPTION   1  !* Default Value *!\n";
  cc_file<<"\n";

  //write general parameters block
  
  //name of the experiment + source + scanname + channel
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"EXPERIMENT       " << Experiment << endl;
  //mode id and index
  cc_file<<"Mode  ID index   " << ModeCh << "  " << ModeChI << endl;
  //channel id and index
  cc_file<<"Channel ID index " << LFT << "  " << ChanChI << endl;
  //start time
  cc_file<<"START            " << 
    VP.ScanStart(VP.Station(StatChI[0]), ScanChI) <<
    " !* Default: later START allowed, but before START + DURATION *!\n";
  //scan duration 
  cc_file<<"DURATION         " << 
    VP.ScanDuration(VP.Station(StatChI[0]), ScanChI) <<
    " !* Default: shorter DURATION allowed *!\n";
  //nr of stations
  cc_file<<"NSTATIONS        " << n_stat_ccf << endl;
  //directory for output
  cc_file<<"OUTDIR           !* REPLACE BY ACTUAL NAME *!\n";
  //logfile name
  cc_file<<"LOGFILE          !* REPLACE BY ACTUAL NAME *!\n";
  //correlation file name
  cc_file<<"CORFILE          !* REPLACE BY ACTUAL NAME *!\n";
  cc_file<<"\n";


  //bandwidth input
  cc_file<<"#_____________________________________________________________\n";
  double skyfreq = 
    VP.SkyFreq(VP.Station(StatChI[0]), ModeCh, ChanChI) * 1000000.;
  cc_file<<"SKYFREQ     " << skyfreq<< endl;
  double bwin =  VP.BW(VP.Station(StatChI[0]),ModeCh,ChanChI);
  cc_file<<"BWIN        "<< bwin <<endl;
  //fft length in delay correction
  cc_file<<"N2FFTDEL    2048   !* Default Value *!\n";
  //delay columns
  cc_file<<"DELCOLS     1 1 1  !* Default Value *!\n";
  cc_file<<"\n";

  //filter switch
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"FILTER      0  !* Default Value *!\n";
  //filter bandwidth
  cc_file<<"BWFL        0  !* Set if FILTER=1 *!\n";
  //filter start frequency
  cc_file<<"STARTF      0  !* Set if FILTER=1 *!\n";
  //filter resolution
  cc_file<<"DELTAF      0  !* Set if FILTER=1 *!\n";
  //filter oversampling
  cc_file<<"OVRFL       1  !* Set if FILTER=1 *!\n";
  cc_file<<"\n";

  //correlation fft length
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"N2FFTCORR   256  !* Default Value *!\n";
  //correlation overlap
  cc_file<<"OVRLP       0.0  !* Default Value *!\n";
  //correlation time to average
  cc_file<<"TIME2AVG    0.5  !* Default Value *!\n";
  //correlation padding
  cc_file<<"PAD         2    !* Default Value *!\n";
  cc_file<<"\n\n";

  //write block for each station
  cout.fill('0');
  for (int i=0; i<n_stat_ccf; i++)
  {
    string StationStr(VP.Station(StatChI[i]));
    int fo=VP.FanOut(StationStr,ModeCh);
    //station_name data_type
    cc_file<<"#_____________________________________________________________\n";
    cc_file << "ST" << setfill('0') << setw(4) << i << "  " <<
      StationStr << " " <<
      VP.TrackFormat(StationStr,ModeCh) << endl << endl;
    //track_bit_rate
    cc_file<<"TBR         " << 2 * bwin / 1000000 / fo << endl;
    //fan_out
    cc_file<<"FO          " << fo << endl;
    //bits_per_sample
    int BPS=VP.N_Bits(StationStr,ModeCh);
    cc_file<<"BPS         " << BPS << endl;
    //nr_of_headstacks
    int NHS=1;
    for (int j=0; j<VP.N_TrackLines(StationStr,ModeCh); j++)
      if (VP.HeadstackNr(StationStr,ModeCh,j)>1) 
        NHS=VP.HeadstackNr(StationStr,ModeCh,j);
    cc_file<<"NHS         " << NHS << endl;
    //byte offset
    cc_file<<"BOFF        0  !* Default Value *!\n";
    //synhs1
    cc_file<<"SYNHS1      1  !* Default Value *!\n";
    //synhs2
    cc_file<<"SYNHS2      2  !* Default Value *!\n";
    //mk4file
    cc_file<<"MK4FILE     !* REPLACE BY ACTUAL NAME *!\n";
    //rndhdr
    cc_file<<"RNDHDR      1  !* Default Value *!\n";

    int trnr=0;
    string RTF,SM;
    //sign
    while (trnr < VP.N_TrackLines(StationStr, ModeCh)) {
      RTF=VP.Resolve_track_freq(StationStr, ModeCh,trnr);
      SM=VP.TrackSignMag(StationStr, ModeCh,trnr);
      if (RTF == LFT && SM=="sign"){
        cc_file<<"SIGN        " << 
          VP.HeadstackNr(StationStr,ModeCh,trnr) << " ";
        for (int j=0; j<fo; j++)
          cc_file << VP.TrackNr(StationStr,ModeCh,trnr*fo+j) << " ";
        cc_file << endl;
        break;
      }
      trnr++;
    }

    //magn
    trnr=0;
    if(BPS==2) {
      while (trnr < VP.N_TrackLines(StationStr, ModeCh)) {
        RTF=VP.Resolve_track_freq(StationStr, ModeCh,trnr);
        SM=VP.TrackSignMag(StationStr, ModeCh,trnr);
        if (RTF == LFT && SM=="mag"){
          cc_file<<"MAGN        " << 
            VP.HeadstackNr(StationStr,ModeCh,trnr) << " ";
          for (int j=0; j<fo; j++)
            cc_file << VP.TrackNr(StationStr,ModeCh,trnr*fo+j) << " ";
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
      VP.ExperName()+"_"+VP.ScanName(ScanChI)+"_"+ 
      VP.Station(StatChI[i])+".del";
    cc_file<<"DELAYTABLE  " << deltbl << endl;
    //DELAYEND
    cc_file<<"DELAYEND\n\n\n";
  }  
  
  //close ccf
  cc_file.close();
  
  //---------------------------------------------------------------------------  
  //create dcf and write proper values (dcf=delay modelcontrol file)

  //set dcf name    
  string dcf_name;
    
  cout << "Enter dcf name or enter A to generate name automatically. \n";
  cin >> dcf_name;
  if (dcf_name=="A") dcf_name=Experiment+".dcf";
  
  //open ccf
  ofstream dc_file(dcf_name.c_str());

  time(&creation_time);
  dc_file<<"#_____________________________________________________________\n";
  dc_file<<"# DCF_REV " << REVISION <<"\n";
  dc_file<<"# Control file for delmo.\n";
  dc_file<<"# Using data from: " << vexStr << "\n";
  dc_file<<"# Generated on: " <<  ctime(&creation_time) << "\n\n";  

  //name of the experiment + source + scanname + channel
  dc_file<<"#EXPERIMENT___________________________________________________\n";
  dc_file<<"experiment      " << Experiment << endl;
  //mode id and index
  dc_file<<"Mode ID         " << ModeCh << endl;
  //scan id
  dc_file<<"Scan ID         " << VP.ScanName(ScanChI) << endl;
  //channel id and index
  dc_file<<"Channel ID      " << LFT << endl;
  //nr of stations
  dc_file<<"nr of stations  " << n_stat_ccf << endl << endl;


  //for each station write relevant parameters to dcf
  int axty=0;
  for (int i=0; i<n_stat_ccf; i++)
  {
    string deltbl = 
      VP.ExperName()+"_"+VP.ScanName(ScanChI)+"_"+ 
      VP.Station(StatChI[i])+".del";

    dc_file<<"#SITE_________________________________________________________\n";
    dc_file<<"DELAYTABLE      "<< deltbl << endl;
    dc_file<<"site_name       "<< VP.Site(VP.Station(StatChI[i]))<<endl;
    dc_file<<"site_position_x "<< VP.SiteX(VP.Station(StatChI[i]))<< endl;
    dc_file<<"site_position_y "<< VP.SiteY(VP.Station(StatChI[i]))<< endl;
    dc_file<<"site_position_z "<< VP.SiteZ(VP.Station(StatChI[i]))<< endl;
    if (VP.AxisMount(VP.Station(StatChI[i])) == "az_el") axty=3;
    if (VP.AxisMount(VP.Station(StatChI[i])) == "ha_dec") axty=4;
    dc_file<<"axis_type       "<< axty << endl;
    dc_file<<"axis_offset     "<< VP.AxisOffset(VP.Station(StatChI[i]))<<endl;
    dc_file<<endl;
  }


  int SourceChI;
  dc_file<<"#SOURCE_______________________________________________________\n";
  for(int j= 0; j<VP.N_Sources(); j++)
    if(VP.SourceName(j) == VP.ScanSource(VP.Station(StatChI[0]),ScanChI))
      SourceChI=j;
  dc_file<<"source_name     " <<  VP.SourceName(SourceChI)<<endl;
  dc_file<<"source_ra       " <<  VP.Source_RA(SourceChI)<<endl;
  dc_file<<"source_dec      " <<  VP.Source_Dec(SourceChI)<<endl;
  dc_file<<endl;

  dc_file<<"#EOP_________________________________________________________\n";
  dc_file<<"tai_utc         " << VP.TAI_UTC()<<endl;
  string eop_ref_epoch=VP.EOPEpoch();
  int year = str_to_long(eop_ref_epoch,0,4);  //pos=0, length=4
  int doy = str_to_long(eop_ref_epoch,5,3);
  int month, day;
  yd2md(year,doy,month,day);
  dc_file<<"eop_ref_epoch   " << JD(year,month,day) <<" julian day"<<endl;
  dc_file<<"num_eop_points  " << VP.N_EOP_Points()<<endl;
  for(int i=0; i<VP.N_EOP_Points(); i++) 
    dc_file<< "ut1_utc[" << i << "]      " << VP.UT1_UTC(i) << endl;
  for(int i=0; i<VP.N_EOP_Points(); i++) 
    dc_file<< "x_wobble[" << i << "]     " << VP.XWobble(i) << endl;
  for(int i=0; i<VP.N_EOP_Points(); i++) 
    dc_file<< "y_wobble[" << i << "]     " << VP.YWobble(i) << endl;
  dc_file<<endl;


  dc_file<<"#SCAN_______________________________________________________\n";
  string startTime=VP.ScanStart(VP.Station(StatChI[0]), ScanChI);
  year = str_to_long(startTime,0,4);  //pos=0, length=4
  doy = str_to_long(startTime,5,3);
  yd2md(year,doy,month,day);
  
  dc_file<<"year            " << year << endl;  //pos=0, length=4
  dc_file<<"month           " << month << endl;
  dc_file<<"day             " << day << endl;
  dc_file<<"hr              " << str_to_long(startTime,9,2) << endl;
  dc_file<<"min             " << str_to_long(startTime,12,2) << endl;
  dc_file<<"sec             " << str_to_long(startTime,15,2) << endl;


  dc_file<<"scan_duration   " << 
    VP.ScanDuration(VP.Station(StatChI[0]), ScanChI)  << endl;
  dc_file<<endl;

  dc_file<<"#RFREQ_______________________________________________________\n";
  dc_file<<"freq            " << skyfreq << endl;
  dc_file<<endl;

  //close ccf
  dc_file.close();

  return 0;
}
