/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * this file contains timer functions
 */

#include "Timer.h"

//default constructor, set default values for run parameters
Timer::Timer()
{
  ID = "unknown process ";
}

//Start a timer
void Timer::start(Log_writer &log_writer)
{
    CPUbegin=clock();
    Tbegin=time(NULL);
    log_writer << "\n\n***** Timer for " << ID << " started.\n\n";
}


//Stop a timer
void Timer::stop(Log_writer &log_writer)
{
    double T_elapsed, CPU_elapsed;
    
    Tend=time(NULL);
    CPUend=clock();
    
    CPU_elapsed=((double) (CPUend - CPUbegin))/CLOCKS_PER_SEC;
    T_elapsed=difftime(Tend, Tbegin);
    log_writer << "\n\n***** Timer for " << ID << " stopped.\n";
    log_writer <<     "***** CPU elapsed " << CPU_elapsed << " seconds Total " << 
      T_elapsed << " seconds\n\n";
}


