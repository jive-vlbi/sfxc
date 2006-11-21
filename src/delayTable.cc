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

//these defines have to be the first in source file
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

//enable define on 32 bit CPU, disable on 64 bit CPU
#define THIRTYTWO

//32 bit machine define,
//use open, lseek, off_t in stead off open64, lseek64, off64_t
#ifdef THIRTYTWO
#define _FILE_OFFSET_BITS 64
#endif

//WARNING, check if definitions are appropriate on machine
//definition of 32 bit and 64 bit (un)signed integers
#define INT32  int
#define UINT32 unsigned int
#define INT64  long long
#define UINT64 unsigned long long

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
delayTable::delayTable()
{
  ndel = 1;
  
  tdt= new INT64  [1]; 

  cA = new double [1];
  cB = new double [1];
  cC = new double [1];

  mA = new double [1];
  mB = new double [1];
  mC = new double [1];
  
  rA = new double [1];
  rB = new double [1];
  rC = new double [1];

  fA = new double [1];
  fB = new double [1];
  fC = new double [1];

}

//destructor
delayTable::~delayTable()
{
   delete [] tdt;

   delete [] cA;
   delete [] cB;
   delete [] cC;

   delete [] mA;
   delete [] mB;
   delete [] mC;

   delete [] rA;
   delete [] rB;
   delete [] rC;

   delete [] fA;
   delete [] fB;
   delete [] fC;

}


//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int delayTable::readDelayTable(char *delayTableName, INT64 start, INT64 stop)
{

  int    retval = 0;
  int    idel;
  FILE   *fp;
  INT64  dt;
  INT64  t0,t1,t2; // time of day in micro seconds
  double c0,c1,c2; // various delay contributions in micro seconds
  double m0,m1,m2;
  double r0,r1,r2;
  double f0,f1,f2;   

  //open delay table
  fp = fopen(DelayTableName, "r");
  if (!fp) {
    cerr << "Error: could not open file :" << DelayTableName << endl;
    return 1;
  }

  //initialisation
  //read line 0, 1, 2 from delay table and assign values to t?, c?, r?, f?
  //where ?={0|1|2]
  retval = getDelayLine(fp, t0, c0, m0, r0, f0);
  retval = retval + getDelayLine(fp, t1, c1, m1, r1, f1);
  retval = retval + getDelayLine(fp, t2, c2, m2, r2, f2);
  if (retval != 0) {
    cerr << "Error: reading data from:" << DelayTableName << endl;
    return retval;
  }
  dt = t2-t1;
  
  //number of parabolic delay functions
  ndel = (start-stop) / dt + 3;
  
  //allocate arrays
  tdt= new INT64  [ndel];
  
  cA = new double [ndel];
  cB = new double [ndel];
  cC = new double [ndel];

  mA = new double [ndel];
  mB = new double [ndel];
  mC = new double [ndel];
  
  rA = new double [ndel];
  rB = new double [ndel];
  rC = new double [ndel];

  fA = new double [ndel];
  fB = new double [ndel];
  fC = new double [ndel];  
  
  //look for start point in delay table
  while (t2 < start)
  {
    t0 = t1;    t1 = t2;
    c0 = c1;    c1 = c2;
    m0 = m1;    m1 = m2;
    r0 = r1;    r1 = r2;
    f0 = f1;    f1 = f2;
    retval = getDelayLine(fp, t2, c2, m2, r2, f2);
    if (retval != 0) {
      cerr << "Error: reading data from:" << DelayTableName << endl;
      return retval;
    }
  }

  idel = 0;
  //fill arrays c?, m?, r?, f? where ?=[A|B|C]
  while ( (t2-stop < dt)  &&  (idel < ndel) )
  {
    //calculate parabolic ceofficients
    tdt[idel]=t1;
    parabCoefs(t0,t1,t2,c1,c2,c3,cA[[idel]],cB[[idel]],cC[[idel]]);
    parabCoefs(t0,t1,t2,m1,m2,m3,mA[[idel]],mB[[idel]],mC[[idel]]);
    parabCoefs(t0,t1,t2,r1,r2,r3,rA[[idel]],rB[[idel]],rC[[idel]]);
    parabCoefs(t0,t1,t2,f1,f2,f3,fA[[idel]],fB[[idel]],fC[[idel]]);
    //process next line
    t0 = t1;    t1 = t2;
    c0 = c1;    c1 = c2;
    m0 = m1;    m1 = m2;
    r0 = r1;    r1 = r2;
    f0 = f1;    f1 = f2;
    retval = getDelayLine(fp, t2, c2, m2, r2, f2);
    if (retval != 0) {
      cerr << "Error: reading data from:" << DelayTableName << endl;
      return retval;
    }
    idel++;
  }
  
  //close delay table
   fclose(fp);

   return retval;
   
}


//calculate the delay for the delayType at time in microseconds
double delayTable::calcDelay(INT64 time, int delayType)
{
   INT64 index;
   //set start time scale to zero
   time = time - tdt[0]; 
   //calculate array index for closest time
   index = (2*time + dt) / (2*dt);
   
   switch delayType {
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
   time = time + tdt[0];
   //calculate delay;
   delay = A*time*time + B*time + C;

   return delay;
}


//get the next line from the delay table file
int getDelayLine(FILE *fp, INT64 &t, double &c, double &m, double &r, double &f)
{
  int   retval = 0;
  char *sep    = " ";
  
  fgets(sB,256,fp);
  t = atof(strtok(sB,sep))*1000000; //from sec to usec
  c = atof(strtok((char*)0,sep)) / 1000000.0; //from usec to sec
  m = atof(strtok((char*)0,sep)) / 1000000.0;
  r = atof(strtok((char*)0,sep)) / 1000000.0;
  f = StaPrms.get_cde()*c + StaPrms.get_mde()*m + StaPrms.get_rde()*r;
  
  return retval;
}


//calculate the parabolic coefficients A, B and C
//delay = A*t*t + B*t + C
int parabCoefs (INT64 t0,INT64 t1,INT64 t2,double d1,double d2,double d3,
  double& A,double& B,double& C)
{
  
  double L,R;

  L = (d0-d1)*(t1-t2) - (d1-d2)*(t0-t1);
  R = (t0*t0-t1*t1)*(t1-t2) - (t1*t1-t2*t2)*(t0-t1);
  A = L/R;
  B = ( (d0-d1) - A*(t0*t0-t1*t1) ) / (t0-t1);
  C = d0-A*t0*t0-b*t0;

  return 0;
}


