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

//default constructor, set default values
Timer::Timer() {
  CPU_elapsed = CPU_accu = 0.0;
  T_elapsed = T_accu = 0.0;
  ID = "unknown process ";
}

//Start a timer
void Timer::start(Log_writer &log_writer) {
  CPUbegin=clock();
  Tbegin=time(NULL);
  log_writer << "\n\n***** Timer for " << ID << " started.\n\n";
}

//Start a timer
void Timer::start() {
  CPUbegin=clock();
  Tbegin=time(NULL);
}


//Stop a timer
void Timer::stop(Log_writer &log_writer) {

  Tend=time(NULL);
  CPUend=clock();

  CPU_elapsed=((double) (CPUend - CPUbegin))/CLOCKS_PER_SEC;
  T_elapsed=difftime(Tend, Tbegin);
  log_writer << "\n\n***** Timer for " << ID << " stopped.\n";
  log_writer <<     "***** CPU elapsed " << CPU_elapsed << " seconds Total " <<
  T_elapsed << " seconds\n\n";
}

//Stop a timer and accumulated timer results
void Timer::stop_accumulate() {

  Tend=time(NULL);
  CPUend=clock();

  CPU_elapsed=((double) (CPUend - CPUbegin))/CLOCKS_PER_SEC;
  T_elapsed=difftime(Tend, Tbegin);

  CPU_accu += CPU_elapsed;
  T_accu   += T_elapsed;
}


//Show accumulated results
void Timer::show_accu_result(Log_writer &log_writer) {
  log_writer << "\n\n***** Timer for " << ID << " stopped.\n";
  log_writer <<     "***** CPU elapsed " << CPU_accu << " seconds Total " <<
  T_accu << " seconds\n\n";
}


