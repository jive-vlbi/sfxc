/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *
 *  This file contains:
 *     - A real-time timer with the same interface as the timer.h. The difference
 *       is that timer.h return the amount of CPU time the process had while
 *       rttimer.h return the real clock wall time.
 */
#ifndef RTTIMER_H
#define RTTIMER_H

#include "utils.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctime>

class RTTimer {
  friend std::ostream& operator<<(std::ostream& os, const RTTimer& t);

private:
  bool running;

  uint64_t start_time;
  uint64_t acc_time;

  uint64_t elapsed_time() const;

  char *name;
public:
  // 'running' is initially false.  A RTTimer needs to be explicitly started
  // using 'start' or 'restart'
  RTTimer()
      : running(false), start_time(0), acc_time(0), name(NULL) {}
  RTTimer(const char * name_)
      : running(false), start_time(0), acc_time(0) {
    int size = strlen(name_);
    name = (char*)malloc((size+1)*sizeof(char));
    strncpy(name, name_, size+1);
  }

  ~RTTimer() {
    if ((measured_time() != 0) && (name != NULL)) {
      std::cout << "RTTimer[" << name << "]: "
      << (*this) << std::endl;
    }
  }

  void start(const char* msg = 0);
  void restart(const char* msg = 0);
  void resume(const char* msg = 0);
  void stop(const char* msg = 0);
  void check(const char* msg = 0);

  double measured_time() const;

private:
  inline void getusec(uint64_t &utime) const {
    struct timeval tv;
    gettimeofday(&tv,0);
    utime=tv.tv_sec*1000000LL;
    utime+=tv.tv_usec;
  }

  static inline uint64_t ticksPerSec(void)  {
    return 1000000;
  }

  static inline uint64_t ticksPerMSec(void)  {
    return 1000000/1000;
  }


  static inline double tickToSec(const uint64_t ticks)  {
    return 1.0*ticks/ticksPerSec();
  }

}
; // class RTTimer

//===========================================================================
// Return the total time that the timer has been in the "running"
// state since it was first "started" or last "restarted".  For
// "short" time periods (less than an hour), the actual cpu time
// used is reported instead of the elapsed time.

inline uint64_t RTTimer::elapsed_time() const {
  uint64_t acc_sec;
  getusec(acc_sec);
  return acc_sec - start_time;
}

//===========================================================================
// Start a timer.  If it is already running, let it continue running.
// Print an optional message.

inline void RTTimer::start(const char* msg) {
// Print an optional message, something like "Starting timer t";
  if (msg) std::cout << msg << std::endl;

// Return immediately if the timer is already running
  if (running) return;

// Set timer status to running and set the start time
  running = true;
  getusec(start_time);

} // RTTimer::start

//===========================================================================
// Turn the timer off and start it again from 0.  Print an optional message.

inline void RTTimer::restart(const char* msg) {
// Print an optional message, something like "Restarting timer t";
  if (msg) std::cout << msg << std::endl;

// Set timer status to running, reset accumulated time, and set start time
  running = true;
  acc_time = 0;
  getusec(start_time);

} // RTTimer::restart

//===========================================================================
// Turn the timer on again.  Print an optional message.

inline void RTTimer::resume(const char* msg) {
// Print an optional message, something like "Restarting timer t";
  if (msg) std::cout << msg << std::endl;

// Return immediately if the timer is already running
  if (running) return;

  // Set timer status to running, reset accumulated time, and set start time
  running = true;
  getusec(start_time);

} // RTTimer::restart

//===========================================================================
// Stop the timer and print an optional message.

inline void RTTimer::stop(const char* msg) {
// Print an optional message, something like "Stopping timer t";
  if (msg) std::cout << msg << std::endl;

// Compute accumulated running time and set timer status to not running
  if (running) acc_time += elapsed_time();
  running = false;

} // RTTimer::stop

//===========================================================================
// Print out an optional message followed by the current timer timing.

inline void RTTimer::check(const char* msg) {
// Print an optional message, something like "Checking timer t";
  if (msg) std::cout << msg << " : ";

  std::cout << "Elapsed time [" << std::setiosflags(std::ios::fixed )
  << std::setprecision(2)
  << tickToSec(acc_time + (running ? elapsed_time() : 0)) << "] seconds\n";

} // RTTimer::check

//===========================================================================
// Allow timers to be printed to ostreams using the syntax 'os << t'
// for an ostream 'os' and a timer 't'.  For example, "cout << t" will
// print out the total amount of time 't' has been "running".

inline std::ostream& operator<<(std::ostream& os, const RTTimer& t) {
  os << std::setprecision(2) << std::setiosflags(std::ios::fixed )
  << RTTimer::tickToSec(t.acc_time + (t.running ? t.elapsed_time() : 0));
  return os;
}

//===========================================================================


inline double RTTimer::measured_time() const {
  return tickToSec(acc_time + (running ? elapsed_time() : 0) );
}
#endif // RTTIMER_H

