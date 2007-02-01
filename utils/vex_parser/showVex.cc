//showVex: test program to show some vex parameters 
//using VexPlus class and the vex parser
//author : RHJ Oerlemans
//date   : 25-01-2007

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//c++ includes
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
    cout << "usage: sV vexfilename <>\n";
    return 0;
  }
    

  const string vexStr(argv[1]);
  
  cout << vexStr << endl;

  //initialize
  VexPlus VP(vexStr);

  //open vex file
  VP.parseVex();

  
  //display some data

  //$GLOBAL
  cout << "\n$GLOBAL\n";
  cout << "ExperName  " << VP.ExperName() << endl;
  
  //$MODE
  cout << "\n$MODE\n";
  int n_modes = VP.N_Modes();
  cout << "N_Modes    " << n_modes << endl;
  for (int i=0; i<n_modes; i++)
    cout << i << ":" << VP.Mode(i) << " ";
  cout << endl;  
    
  string ModeCh(VP.Mode(0));
  if (n_modes > 1) {
    cout << "Enter mode number of your choice\n";
    int ModeChI;
    cin >> ModeChI;
    ModeCh=VP.Mode(ModeChI);
  }  

  //$STATION
  cout << "\n$STATION\n";
  int n_stations = VP.N_Stations();
  cout << "N_Stations " << n_stations << endl;
  cout << "Stations   ";
  for (int i=0; i < n_stations; i++)
    cout << i << ":" << VP.Station(i) << " ";
  cout << endl;
  string StatCh(VP.Station(0));
  if (n_stations > 1) {
    cout << "Enter station number of your choice\n";
    int StatChI;
    cin >> StatChI;
    StatCh=VP.Station(StatChI);
  }  

  //$FREQ
  cout << "\n$FREQ\n";
  cout << "Mode=" << ModeCh << "  Station=" << StatCh << endl;
  cout << "N_FreqChans  " << VP.N_FreqChans(StatCh,ModeCh) << endl;
  cout << "SampleRate   " << VP.SampleRate(StatCh,ModeCh) << endl;
  cout << "ChId   Sky_freqs  BW\n";
  for (int i=0; i<VP.N_FreqChans(StatCh,ModeCh); i++)
    cout <<
    VP.Link_freq_track(StatCh,ModeCh,i) << "  " <<
    VP.SkyFreq(StatCh,ModeCh,i) << "  " <<
    VP.BW(StatCh,ModeCh,i)<< endl;
  cout << endl;

  //$IF
  cout << "\n$IF\n";
  cout << "Mode=" << ModeCh << endl;
  cout << "StId  IfId  Pol  LO  \n";
  for (int i=0; i<n_stations; i++){
    for (int j=0; j<VP.N_IFs(VP.Station(i),ModeCh); j++){
      cout <<
      VP.Station(i) << "  " <<
      VP.Resolve_if_bbc(VP.Station(i),ModeCh,j) << "  " <<
      VP.Pol(VP.Station(i),ModeCh,j) << "  " <<
      VP.LO(VP.Station(i),ModeCh,j) << endl;
    }
  }
  
  //$TRACKS
  cout << "\n$TRACKS\n";
  cout << "Mode=" << ModeCh << "  Station=" << StatCh << endl;
  cout << "N_TrackLines  " << VP.N_TrackLines(StatCh,ModeCh) << endl;
  cout << "N_Bits        " << VP.N_Bits(StatCh,ModeCh) << endl;
  int fo = VP.FanOut(StatCh,ModeCh);
  cout << "FanOut        " << fo << endl;
  cout << "TrackFormat   " << VP.TrackFormat(StatCh,ModeCh) << endl;
  cout << "Modulation    " << VP.Modulation(StatCh,ModeCh) << endl;
  cout << "ChId  S/M  HS  trcks \n";
  for (int i=0; i<VP.N_TrackLines(StatCh,ModeCh); i++){
    cout <<
    VP.Resolve_track_freq(StatCh,ModeCh,i) << " " <<
    VP.TrackSignMag(StatCh,ModeCh,i) << " " <<
    VP.HeadstackNr(StatCh,ModeCh,i) << " ";
    for (int j=0; j < fo; j++)
      cout << VP.TrackNr(StatCh,ModeCh,i*fo+j) << " " ;
    cout<< endl;
  }
  
  
  //$SOURCES
  cout << "\n$SOURCES\n";
  cout << "N_Sources  " << VP.N_Sources() << endl;

  //$SCHED
  cout << "\n$SCHED\n";
  cout << "  Station=" << StatCh << endl;
  int n_scans = VP.N_Scans(StatCh);
  cout << "N_Scans      " << n_scans << endl;
  cout << "Scan  Start  Source  Duration\n";
  for (int i=0; i<n_scans; i++)
    cout <<
    VP.ScanName(i) << "  " <<
    VP.ScanStart(StatCh,i) << "  " <<
    VP.ScanSource(StatCh,i) <<  "  " <<
    VP.ScanDuration(StatCh,i) << endl;

  
  return 0;
}
