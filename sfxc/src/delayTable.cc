/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class function definitions for station specific data
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

bool DelayTable::operator==(const DelayTable &other) const
{
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
    
  return true;
}


void DelayTable::reserve_data() 
{
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


//set values for delay table columns
void DelayTable::set_cmr(GenP GenPrms)
{
  cde = GenPrms.get_cde();
  mde = GenPrms.get_mde();  
  rde = GenPrms.get_rde();  
}


//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int DelayTable::readDelayTable(char *delayTableName)
{
  int    retval = 0;
  FILE   *fp;
  int64_t  t0,t1,t2; // time of day in micro seconds
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
    cerr << "ERROR: reading data from: " << delayTableName << endl;
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



//calculates the delay for the delayType at time in microseconds
//get the next line from the delay table file
double DelayTable::calcDelay(int64_t time, int delayType) const
{
  int64_t index;
  double A,B,C,delay;
   
   //set start time scale to zero
   time = time - startDT;
   //calculate array index for closest time
   index = (2*time + stepDT) / (2*stepDT);
   if ((index < 0) || (index >= ndel)) {
     cout.precision(20);
     cout << "time=" << time << " index=" << index << " ndel=" << ndel <<endl;
     assert(false);
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


//gets one line from the delay file, where;
//t: time
//c: correlator delay model
//m: tangential motion model
//r: radial motion model
//f: sum of c, m, r
int DelayTable::getDelayLine(FILE *fp, int64_t &t, 
  double &c, double &m, double &r, double &f)
{
  int   retval = 0;
  char *sep    = " ";
  char sB[512];
  
  if (feof(fp)) return 1;

  if (fgets(sB,256,fp) == NULL) return 1;
  t = (int64_t)(atof(strtok(sB,sep))*1000000); //from sec to usec
  c = atof(strtok((char*)0,sep)); 
  m = atof(strtok((char*)0,sep));
  r = atof(strtok((char*)0,sep));
  f = cde*c + mde*m + rde*r;
  
  return retval;
}


//calculate the parabolic coefficients A, B and C
//delay = A*t*t + B*t + C
int DelayTable::parabCoefs (int64_t t0,int64_t t1,int64_t t2,
  double d0,double d1,double d2,
  double& A,double& B,double& C)
{
  double L,R;

  assert((t0 != t1) && (t1 != t2));
  L = (d0-d1)*(t1-t2) - (d1-d2)*(t0-t1);
  R = (t0*t0-t1*t1)*(t1-t2) - (t1*t1-t2*t2)*(t0-t1);
  A = L/R;
  B = ( (d0-d1) - A*(t0*t0-t1*t1) ) / (t0-t1);
  C = d0-A*t0*t0-B*t0;
  assert(L==L);
  assert(R==R);
  assert(A==A);
  assert(B==B);
  assert(C==C);

  return 0;
}


