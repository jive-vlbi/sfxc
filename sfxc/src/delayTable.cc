/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class function definitions for station specific data

Author     : RHJ Oerlemans
StartDate  : 20061117
Last change: 20061121

*/

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "delayTable.h"

//global variables
//declaration and default settings run parameters
extern RunP RunPrms;
//declaration and default settings general parameters
extern GenP GenPrms;
//station parameters class, declaration and default settings
extern StaP StaPrms[NstationsMax];

//*****************************************************************************
//function prototypes for local functions
//*****************************************************************************

//get the next line from the delay table file
int getDelayLine(FILE *fp, INT64 &t, double &c, double &m, double &r, double &f);

//calculate the parabolic coefficients A, B and C
int parabCoefs (INT64 t0,INT64 t1,INT64 t2,double d1,double d2,double d3,
  double& A,double& B,double& C);

//*****************************************************************************
//function definitions
//*****************************************************************************

//default constructor, set default values
DelayTable::DelayTable() : ndel(1)
{
  reserve_data();  
}

//destructor
DelayTable::~DelayTable()
{
}

bool 
DelayTable::operator==(const DelayTable &other) const {
  if (ndel != other.ndel) return false;
  if (startDT != other.startDT) return false;
  if (stepDT != other.stepDT) return false;
  
  if (cA != other.cA) return false;
  if (cB != other.cB) return false;
  if (cC != other.cC) return false;

  if (mA != other.mA) return false;
  if (mB != other.mB) return false;
  if (mC != other.mC) return false;

  if (rA != other.rA) return false;
  if (rB != other.rB) return false;
  if (rC != other.rC) return false;

  if (fA != other.fA) return false;
  if (fB != other.fB) return false;
  if (fC != other.fC) return false;
  
  double t = startDT, delay1, delay2;
  while (t < startDT + (ndel-2) *stepDT) {
    //Cdel, Mdel, Rdel, Fdel
    delay1 = calcDelay(t, Cdel);
    delay2 = other.calcDelay(t, Cdel);
    assert(delay1 == delay2);
    t += .02*stepDT;
  }
  
  return true;
}

void DelayTable::reserve_data() {
  cA.resize(ndel);
  cB.resize(ndel);
  cC.resize(ndel);

  mA.resize(ndel);
  mB.resize(ndel);
  mC.resize(ndel);
  
  rA.resize(ndel);
  rB.resize(ndel);
  rC.resize(ndel);

  fA.resize(ndel);
  fB.resize(ndel);
  fC.resize(ndel);
}
//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int DelayTable::readDelayTable(char *delayTableName,
  INT64 start, INT64 stop, INT64 BufTime)//time in usec
{

  int    retval = 0;
  int    idel;
  FILE   *fp;
  INT64  t0,t1,t2; // time of day in micro seconds
  double c0,c1,c2; // various delay contributions in micro seconds
  double m0,m1,m2;
  double r0,r1,r2;
  double f0,f1,f2;   

  //open delay table
  fp = fopen(delayTableName, "r");
  if (!fp) {
    cerr << "Error: could not open file :" << delayTableName << endl;
    return 1;
  }

  //initialisation
  //read line 0, 1, 2 from delay table and assign values to t?, c?, r?, f?
  //where ?={0|1|2]
  retval =          getDelayLine(fp, t0, c0, m0, r0, f0);
  retval = retval + getDelayLine(fp, t1, c1, m1, r1, f1);
  retval = retval + getDelayLine(fp, t2, c2, m2, r2, f2);
  if (retval != 0) {
    cerr << "ERROR: reading data from:" << delayTableName << endl;
    return retval;
  }
  stepDT = t2-t1;
  if (2*stepDT <= BufTime) {
    cerr << "ERROR: 2*step time in delay table < BufTime." << endl;
    return 1;
  }
  
  //number of parabolic delay functions
  ndel = (stop - start) / stepDT + 3;
  //allocate arrays
  reserve_data();
  
  //look for start point in delay table
  while ( !(t2 > start) )
  {
    t0 = t1;    t1 = t2;
    c0 = c1;    c1 = c2;
    m0 = m1;    m1 = m2;
    r0 = r1;    r1 = r2;
    f0 = f1;    f1 = f2;
    retval = getDelayLine(fp, t2, c2, m2, r2, f2);
    if (retval != 0) {
      cerr << "Error: reading data from:" << delayTableName << endl;
      return retval;
    }
  }


  idel = 0;
  startDT = t1;
  //fill arrays c?, m?, r?, f? where ?=[A|B|C]
  while ( idel < ndel )
  {
    //calculate parabolic coefficients
    parabCoefs(t0,t1,t2,c0,c1,c2,cA[idel],cB[idel],cC[idel]);
    parabCoefs(t0,t1,t2,m0,m1,m2,mA[idel],mB[idel],mC[idel]);
    parabCoefs(t0,t1,t2,r0,r1,r2,rA[idel],rB[idel],rC[idel]);
    parabCoefs(t0,t1,t2,f0,f1,f2,fA[idel],fB[idel],fC[idel]);
    //process next line
    t0 = t1;    t1 = t2;
    c0 = c1;    c1 = c2;
    m0 = m1;    m1 = m2;
    r0 = r1;    r1 = r2;
    f0 = f1;    f1 = f2;
    retval = getDelayLine(fp, t2, c2, m2, r2, f2);
    if (retval != 0) {
      cerr << "Error: reading data from:" << delayTableName << endl;
      return retval;
    }
    idel++;
  }
  
  //close delay table
  fclose(fp);

  return retval;
   
}

//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int DelayTable::readDelayTable(char *delayTableName, INT64 BufTime)
{
  int    retval = 0;
  FILE   *fp;
  INT64  t0,t1,t2; // time of day in micro seconds
  double c0,c1,c2; // various delay contributions in micro seconds
  double m0,m1,m2;
  double r0,r1,r2;
  double f0,f1,f2;   

  //open delay table
  fp = fopen(delayTableName, "r");
  if (!fp) {
    cerr << "Error: could not open file :" << delayTableName << endl;
    return 1;
  }

  //initialisation
  //read line 0, 1, 2 from delay table and assign values to t?, c?, r?, f?
  //where ?={0|1|2]
  retval =          getDelayLine(fp, t0, c0, m0, r0, f0);
  retval = retval + getDelayLine(fp, t1, c1, m1, r1, f1);
  retval = retval + getDelayLine(fp, t2, c2, m2, r2, f2);
  if (retval != 0) {
    cerr << "ERROR: reading data from:" << delayTableName << endl;
    return retval;
  }
  
  stepDT  = t2-t1;
  startDT = t1;
  
  //look for start point in delay table
  do {
    double A,B,C;
    //calculate parabolic coefficients
    parabCoefs(t0,t1,t2,c0,c1,c2,A,B,C);
    cA.push_back(A); cB.push_back(B); cC.push_back(C);
    parabCoefs(t0,t1,t2,m0,m1,m2,A,B,C);
    mA.push_back(A); mB.push_back(B); mC.push_back(C);
    parabCoefs(t0,t1,t2,r0,r1,r2,A,B,C);
    rA.push_back(A); rB.push_back(B); rC.push_back(C);
    parabCoefs(t0,t1,t2,f0,f1,f2,A,B,C);
    fA.push_back(A); fB.push_back(B); fC.push_back(C);

    t0 = t1;    t1 = t2;
    c0 = c1;    c1 = c2;
    m0 = m1;    m1 = m2;
    r0 = r1;    r1 = r2;
    f0 = f1;    f1 = f2;
  } while (getDelayLine(fp, t2, c2, m2, r2, f2) == 0);
  
  ndel = cA.size();
  //close delay table
  fclose(fp);

  return 0;
}


//calculate the delay for the delayType at time in microseconds
//get the next line from the delay table file
double DelayTable::calcDelay(double time, int delayType) const
{
  INT64 index;
  double A,B,C,delay;
   
   //set start time scale to zero
   time = time - startDT;
   //calculate array index for closest time
   index = (2*(INT64)time + stepDT) / (2*stepDT);
   if ((index < 0) || (index >= ndel)){
     cout << "time=" << time << " index=" << index << " ndel=" << ndel <<endl;
     return 1.0;
   }  
   switch (delayType) {
     case Cdel:
       A = cA[index];
       B = cB[index];
       C = cC[index];
       break;
     case Mdel:
       A = mA[index];
       B = mB[index];
       C = mC[index];
       break;
     case Rdel:
       A = rA[index];
       B = rB[index];
       C = rC[index];
       break;
     case Fdel:
       A = fA[index];
       B = fB[index];
       C = fC[index];
       break;
     default:
       cerr << "Non existing delay type chosen" << endl;
       return 1.0; //delay should be negative,
                   //positive value indicates error
   }
   //reset time scale
   time = time + startDT;
   //return calculated delay;
   delay = A*time*time + B*time + C;
   return delay;
}


int getDelayLine(FILE *fp, INT64 &t, double &c, double &m, double &r, double &f)
{
  int   retval = 0;
  char *sep    = " ";
  char sB[512];
  
  if (feof(fp)) return 1;

  if (fgets(sB,256,fp) == NULL) return 1;
  t = (INT64)(atof(strtok(sB,sep))*1000000); //from sec to usec
  c = atof(strtok((char*)0,sep)) / 1000000.0; //from usec to sec
  m = atof(strtok((char*)0,sep)) / 1000000.0;
  r = atof(strtok((char*)0,sep)) / 1000000.0;
  f = GenPrms.get_cde()*c + GenPrms.get_mde()*m + GenPrms.get_rde()*r;
  
  return retval;
}


//calculate the parabolic coefficients A, B and C
//delay = A*t*t + B*t + C
int parabCoefs (INT64 t0,INT64 t1,INT64 t2,double d0,double d1,double d2,
  double& A,double& B,double& C)
{
  double L,R;

  L = (d0-d1)*(t1-t2) - (d1-d2)*(t0-t1);
  R = (t0*t0-t1*t1)*(t1-t2) - (t1*t1-t2*t2)*(t0-t1);
  A = L/R;
  B = ( (d0-d1) - A*(t0*t0-t1*t1) ) / (t0-t1);
  C = d0-A*t0*t0-B*t0;

  return 0;
}


