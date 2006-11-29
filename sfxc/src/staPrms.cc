/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class function definitions for station specific data

Author     : RHJ Oerlemans
StartDate  : 20060913
Last change: 20061114

*/

#include <types.h>


//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//the class definitions and function definitions
#include "constPrms.h"
#include "runPrms.h"
#include "staPrms.h"
#include "genFunctions.h"

//global variables
extern RunP  RunPrms;

//*****************************************************************************
//funtion definitions
//*****************************************************************************

//get functions
char* StaP::get_stname()     { return stname; }
char* StaP::get_datatype()   { return datatype; }
int   StaP::get_tbr()        { return tbr; }
int   StaP::get_fo()         { return fo; }
int   StaP::get_bps()        { return bps; }
int   StaP::get_nhs()        { return nhs; }
int   StaP::get_tphs()       { return tphs; }
int   StaP::get_boff()       { return boff; }
int   StaP::get_synhs1()     { return synhs1; }
int   StaP::get_synhs2()     { return synhs2; }
int   StaP::get_mod()        { return mod; }
int   StaP::get_rndhdr()     { return rndhdr; }
char* StaP::get_mk4file()    { return mk4file; }
char* StaP::get_hdrmap()     { return hdrmap; }
char* StaP::get_modpat()     { return modpat; }
char* StaP::get_delaytable() { return delaytable; }
char* StaP::get_phasetable() { return phasetable; }
int   StaP::get_loobs()      { return loobs;}
int*  StaP::get_signBS()     { return signBS;}
int*  StaP::get_magnBS()     { return magnBS;}



//default constructor, set default values
//for station specific control parameters
StaP::StaP()
{
  int j;
  char *home;

  tbr     =  8;
  fo      =  4;
  bps     =  2;
  nhs     =  2;
  tphs    = 32;
  boff    =  0;
  synhs1  =  1;
  synhs2  =  1;
  mod     =  0;
  rndhdr  =  1;
  stname     = new char[256];
  datatype   = new char[256];
  mk4file    = new char[256];
  hdrmap     = new char[256];
  modpat     = new char[256];
  delaytable = new char[256];
  phasetable = new char[256];
  
  strcpy(stname,"undefined");
  strcpy(datatype,"undefined");
  strcpy(mk4file,"undefined");
  if ( ( home = getenv("HOME") )  == NULL){
    strcpy(hdrmap,"header_map_mk4_cx.txt"); //header map file
    strcpy(modpat,"modpattern.txt"); //modulation pattern file
  } else {
    strcpy(hdrmap,home);
    strcat(hdrmap,"/bin/header_map_mk4_cx.txt");
    strcpy(modpat,home);
    strcat(modpat,"/bin/modpattern.txt"); //modulation pattern file
  }
  strcpy(delaytable,"undefined");
  strcpy(phasetable,"undefined");

  hs=hm=-1;
  for (j=0; j<fomax; j++) {
    signBS[j]=magnBS[j]=-1;
  }

}


//destructor
StaP::~StaP() {
  delete [] stname;
  delete [] datatype;
  delete [] mk4file;
  delete [] hdrmap;
  delete [] modpat;
  delete [] delaytable;
  delete [] phasetable;
}


//parse control file C
int StaP::parse_ctrlFile(char *ctrlFile, int staNr)
{
  int  retval=0;
  FILE *ctrlP;
  char *line, *key, *val, *val1, staKW[256];

  //allocate memory
  line     = new char[lineLength];
  key      = new char[lineLength];
  val      = new char[lineLength];
  val1     = new char[lineLength];

  if( access(ctrlFile, R_OK) != 0 ) {
    cerr << "ERROR: File " << ctrlFile <<
      " is not accessible or does not exist.\n";
    return -1;
  }

  //open control file
  ctrlP = fopen(ctrlFile,"r");

  //construct station keyword
  sprintf(staKW,"ST%.4d",staNr);
  
  while (1) {
    //get next line until station keyword is found
    if (fgets(line,lineLength,ctrlP) == (char *) NULL) break;
    //split line contents in key and values
    if (sscanf(line,"%s %s %s\n",key,val,val1) == 3) {
      //check if keyword is station keyword
      if (strcmp(key,staKW) == 0) {
        //station keyword found
        strcpy(stname,val);
        strcpy(datatype,val1);
        if (strcmp(datatype,"MK4") == 0) {
          //look for MK4 type data
          retval = findMK4data(ctrlP);
        }
        //look for Delay data
        retval = findDelaydata(ctrlP);
      }
    }    
  }//end while loop station keyword

  // close control file
  fclose(ctrlP);

  //release memory
  delete [] line;
  delete [] key;
  delete [] val;
  delete [] val1;

  return retval;
}


//look for MK4 type data
int StaP::findMK4data(FILE *ctrlP)
{
  int retval = 0, vall, i;
  char *line, *key, *val, *copy, *s;

  //allocate memory
  line     = new char[lineLength];
  key      = new char[lineLength];
  val      = new char[lineLength];
  copy     = new char[lineLength];

  //initialize key
  strcpy (key,"continue");
  
  while ( strcmp(key,"MK4END")!= 0 ) {
    //look for MK4 type data
    if (fgets(line,lineLength,ctrlP) == (char *) NULL) break;
    //split line contents in key and value
    if (sscanf(line,"%s %s\n",key,val) == 2) {
      //parse the MK4 type data
      retval = retval + getLongVal(key,val,"TBR",tbr);
      retval = retval + getLongVal(key,val,"FO",fo);
      retval = retval + getLongVal(key,val,"BPS",bps);
      retval = retval + getLongVal(key,val,"NHS",nhs);
      retval = retval + getLongVal(key,val,"TPHS",tphs);
      retval = retval + getLongVal(key,val,"BOFF",boff);
      retval = retval + getLongVal(key,val,"SYNHS1",synhs1);
      retval = retval + getLongVal(key,val,"SYNHS2",synhs2);
      retval = retval + getLongVal(key,val,"MOD",mod);
      retval = retval + getLongVal(key,val,"RNDHDR",rndhdr);

      if (strcmp(key,"MK4FILE") == 0) strcpy(mk4file,val);
      if (strcmp(key,"HDRMAP") == 0)  strcpy(hdrmap,val);
      if (strcmp(key,"MODPAT") == 0)  strcpy(modpat,val);

      if( strcmp(key,"SIGN") == 0 ) {
        strcpy(copy,line);
        s = strtok(copy," ");
        s = strtok((char*)0," ");
        retval = retval + str2int(s,vall);//convert to long
        hs = vall;

        i=0;
        s = strtok((char*)0," ");
        while (s != 0 && i< fo) {
          retval = retval + str2int(s,vall);//convert to long
          signBS[i] = vall;
          s = strtok((char*)0," ");
          i++;
        }
      }

      if(strcmp(key,"MAGN") == 0 ) {
        strcpy(copy,line);
        s = strtok(copy," ");
        s = strtok((char*)0," ");
        retval = retval + str2int(s,vall);//convert to long
        hm = vall;

        i=0;
        s = strtok((char*)0," ");
        while (s != 0 && i< fo) {
          retval = retval + str2int(s,vall);//convert to long
          magnBS[i] = vall;
          s = strtok((char*)0," ");
          i++;
        }
      }
    }
  }//end while loop MK4 type data
  
  if (retval != 0) cerr << "ERROR in findMK4data!\n";
  return retval;
}


//look for Delay data
int StaP::findDelaydata(FILE *ctrlP)
{
  int retval = 0;
  char *line, *key, *val;

  //allocate memory
  line     = new char[lineLength];
  key      = new char[lineLength];
  val      = new char[lineLength];

  //initialize key
  strcpy (key,"continue");
  
  while ( strcmp(key,"DELAYEND")!= 0 ) {
    //look for MK4 type data
    if (fgets(line,lineLength,ctrlP) == (char *) NULL) break;
    //split line contents in key and value
    if (sscanf(line,"%s %s\n",key,val) == 2) {
      if (strcmp(key,"DELAYTABLE") == 0) strcpy(delaytable,val);
      if (strcmp(key,"PHASETABLE") == 0) strcpy(phasetable,val);
      retval = retval + getLongVal(key,val,"LOOBS",loobs);
    }
  }
    
  if (retval != 0) cerr << "ERROR in findDelayData!\n";
  return retval;
}


//check the station paramters and optionally display them
int StaP::check_params()
{

  int retval = 0, j;
  if (RunPrms.get_messagelvl() > 0) {
    cout << endl <<
      "MK4 data file        = " << mk4file << "\n" <<
      "Track bit rate       = " << tbr << "Mb/s/track\n" <<
      "Fan out              = 1:" <<fo << "\n" <<
      "Bits per sample      = " << bps << "\n" <<
      "Nr of headstacks     = " << nhs << "\n" <<
      "Tracks per headstack = " << tphs << "\n" <<
      "Fine offset          = " << boff << " bytes\n" <<
      "Sync track headstack1= " << synhs1 << "\n" <<
      "Sync track headstack2= " << synhs2 << "\n" <<
      "Modulation on if 1   = " << mod << "\n" <<
      "Random header if 1   = " << rndhdr << "\n" <<
      "Header map file      = " << hdrmap << "\n";
    if (mod) cout << "Modulation pattern file = "<< mod << "\n";
    cout << endl;

         
    cout << endl <<
      "Delaytable           = " << delaytable << endl <<
      "Phasetable           = " << phasetable << endl <<
      "Local oscilator      = " << loobs << endl;
    cout << endl;
   
    cout << "Mk4 tape bit shift numbers:" << endl;
    
    cout << "SIGN " << hs;
    for (j=0; j < fo; j++) cout <<  " " << signBS[j];
    cout << endl;

    cout << "MAGN " << hm;
    for (j=0; j < fo; j++) cout <<  " " << magnBS[j];
    cout << endl;

  }

  //mk4 to mk5 bit shift numbers
  //recalculation because on tape 36 tracks and track 0,1,34,35 not used on disk
  hs = hs - 1;
  hm = hm - 1;
  for (j=0; j < fo; j++) {
    signBS[j] = signBS[j] - 2 + 32*hs;
    magnBS[j] = magnBS[j] - 2 + 32*hm;
  }
  
  if (RunPrms.get_messagelvl() > 0) {
    cout << "Converted to Mk5 disk bit shift numbers:" << endl;
    
    cout << "SIGN " << hs;
    for (j=0; j < fo; j++) cout <<  " " << signBS[j];
    cout << endl;

    cout << "MAGN " << hm;
    for (j=0; j < fo; j++) cout <<  " " << magnBS[j];
    cout << endl;
  }

  //check control parameters and show faulty ones
  if (!(fo==1 || fo==2 || fo==4 )) {
    cerr << "Fan out = 1: " << fo << ". It should be 1 or 2 or 4\n";
    retval=-1;
  }

  if (!(bps==1 || bps==2)) {
    cerr << "Bits per sample = " << bps << ". It should be 1 or 2\n";
    retval=-1;
  }

  if (!(nhs==1 || nhs==2)) {
    cerr << "Number of headstacks = " << nhs << ". It should be 1 or 2\n";
    retval=-1;
  }

  if (!(tphs==32)) {
    cerr << "Number of tracks per headstack = " << tphs << ". It should be 32\n";
    retval=-1;
  }


  if (boff <0 ) {
    cerr << "boff should be 0 or 1.\n";
    retval=-1;
  }

  if (!(0<=synhs1 && synhs1<32)) {
    cerr << "Synchronisation track for headstack 1 = " << synhs1
         << ". It should be  0<= SYNHS1 < 32\n";
    retval=-1;
  }

  if (!(0<=synhs2 && synhs2<32)) {
    cerr << "Synchronisation track for headstack 1 = " << synhs2
         << ". It should be  0<= SYNHS2 < 32\n";
    retval=-1;
  }

  //check if the data file exists
  if( access(mk4file, R_OK) != 0 ) {
    cerr << "MK4 file " << mk4file << " is not accessible or does not exist\n";
    retval=-1;
  }

  //check if the header map file exists
  if(access(hdrmap,R_OK) != 0 ) {
    cerr << "Header map file " << hdrmap << " is not accessible or does not exist\n";
    retval=-1;
  }

  //check if the modulation pattern file exists only if modulation is applied
  if (mod) {
    if( access(modpat, R_OK) !=0 ) {
      cerr << "Modulation file " << modpat << " is not accessible or does not exist\n";
      retval=-1;
    }
  }

  //check if the delay table exists
  if( access(delaytable, R_OK) != 0 ) {
    cerr << "Delay file " << delaytable << " is not accessible or does not exist\n";
    retval=-1;
  }
  
  return retval;
}



