//-----------------------------------------------------------------------------
//vex2ccf: program to extract some vex parameters
//         and write them into sfxc type correlator control file (ccf)
//         other ccf parameters are set to default values
//         manual editing of ccf required before correlation
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

int main (int argc, char *argv[])
{

  if (argc != 2){
    cout << "purpose: create ccf for sfxc by extracting relevant vex parameters\n";
    cout << "         and setting defaults values for the others.\n";
    cout << "usage  : vex2ccf vexfilename <ret>\n";
    return 0;
  }

  const string vexStr(argv[1]);
  
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
  if (n_modes > 1) {
    int ModeChI;
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

  //extraction of vex data and write default data for other paramters

  //set ccf name    
  string Experiment = VP.ExperName()+"_"+
    VP.ScanSource(VP.Station(StatChI[0]),ScanChI)+"_"+
    VP.ScanName(ScanChI)+"_"+LFT;
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
  cc_file<<"# 3) Only change parameters marked by !* *!\n\n";
  

  //write run parameters block, default values
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"MESSAGELVL  1  !* Default Value *!\n";
  cc_file<<"INTERACTIVE 0  !* Default Value *!\n";
  cc_file<<"RUNOPTION   1  !* Default Value *!\n";
  cc_file<<"\n";

  //write general parameters block
  
  //name of the experiment + source + scanname + channel
  cc_file<<"#_____________________________________________________________\n";
  cc_file<<"EXPERIMENT  " << Experiment << endl;
  //start time
  cc_file<<"START       " << VP.ScanStart(VP.Station(StatChI[0]), ScanChI) <<
    " !* Default: later START allowed, but before START + DURATION *!\n";
  //scan duration
  cc_file<<"DURATION    " << VP.ScanDuration(VP.Station(StatChI[0]), ScanChI) <<
    " !* Default: shorter DURATION allowed *!\n";
  //nr of stations
  cc_file<<"NSTATIONS   " << n_stat_ccf << endl;
  //directory for output
  cc_file<<"OUTDIR      !* REPLACE BY ACTUAL NAME *!\n";
  //logfile name
  cc_file<<"LOGFILE     !* REPLACE BY ACTUAL NAME *!\n";
  //correlation file name
  cc_file<<"CORFILE     !* REPLACE BY ACTUAL NAME *!\n";
  cc_file<<"\n";


  //bandwidth input
  cc_file<<"#_____________________________________________________________\n";
  double bwin=VP.BW(VP.Station(StatChI[0]),ModeCh,ChanChI);
  cc_file<<"BWIN        "<< bwin <<endl;
  //fft length in delay correction
  cc_file<<"N2FFTDEL    2048   !* Default Value *!\n";
  //frequency offset
  cc_file<<"FOFFSET     0      !* Default Value *!\n";
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
  cc_file<<"N2FFTCOR    256  !* Default Value *!\n";
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
      if (VP.HeadstackNr(StationStr,ModeCh,j)>1) NHS=VP.HeadstackNr(StationStr,ModeCh,j);
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
        cc_file<<"SIGN        " << VP.HeadstackNr(StationStr,ModeCh,trnr) << " ";
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
          cc_file<<"MAGN        " << VP.HeadstackNr(StationStr,ModeCh,trnr) << " ";
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
    cc_file<<"DELAYTABLE  !* REPLACE BY ACTUAL NAME *!\n";
    //local_oscilator
    cc_file<<"LOOBS       "<<VP.LO(StationStr,ModeCh,0) << endl;
    //DELAYEND
    cc_file<<"DELAYEND\n\n\n";
  }  
    
  
  //close ccf
  cc_file.close();
  
  return 0;
}
