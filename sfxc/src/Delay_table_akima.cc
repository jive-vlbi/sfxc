/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans  <Oerlemans@JIVE.nl>, 2007
 *            Nico Kruithof   <Kruithof@JIVE.nl>, 2007
 *            Huseyin Ozdemir <Ozdemir@JIVE.nl>, 2007
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
#include "Delay_table_akima.h"

//*****************************************************************************
//function definitions
//*****************************************************************************

//default constructor, set default values
Delay_table_akima::Delay_table_akima()
{
}

//destructor
Delay_table_akima::~Delay_table_akima()
{
}

bool Delay_table_akima::operator==(const Delay_table_akima &other) const
{
    
  return true;
}

//set values for delay table columns
void Delay_table_akima::set_cmr(GenP GenPrms)
{
  cde = GenPrms.get_cde();
  mde = GenPrms.get_mde();  
  rde = GenPrms.get_rde();  
}


//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int Delay_table_akima::open(char *delayTableName)
{
  std::ifstream in(delayTableName);
  double time, delay, tmp1, tmp2;
  while (in >> time >> delay >> tmp1 >> tmp2) {
    // The time read from file is in seconds, whereas the software correlator
    // works with times in microseconds
    times.push_back(time*1000000);
    delays.push_back(delay);
  }

  // Initialise the Akima spline
  acc = gsl_interp_accel_alloc ();
  splineakima = gsl_spline_alloc (gsl_interp_akima, times.size());
  gsl_spline_init(splineakima, &times[0], &delays[0], times.size());
  
  return 0;
}



//calculates the delay for the delayType at time in microseconds
//get the next line from the delay table file
double Delay_table_akima::delay(int64_t time, int delayType) const
{
   return gsl_spline_eval (splineakima, time, acc);
}
