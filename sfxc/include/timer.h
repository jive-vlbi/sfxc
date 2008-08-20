#ifndef TIMER_H
#define TIMER_H

#include "utils.h"

#include <ctime>
#include <iostream>
#include <iomanip>

class Timer {
  friend std::ostream& operator<<(std::ostream& os, const Timer& t);

private:
  bool running;
  clock_t start_clock;
  time_t start_time;
  double acc_time;

  double elapsed_time() const;

  char *name;

public:
  // 'running' is initially false.  A Timer needs to be explicitly started
  // using 'start' or 'restart'
  Timer()
      : running(false), start_clock(0), start_time(0), acc_time(0), name(NULL) {}
  Timer(const char * name_)
      : running(false), start_clock(0), start_time(0), acc_time(0) {
    int size = strlen(name_);
    name = (char*)malloc((size+1)*sizeof(char));
    strncpy(name, name_, size+1);
  }

  ~Timer() {
    if ((measured_time() != 0) && (name != NULL)) {
      std::cout
      << "Timer["<< RANK_OF_NODE << ", " << name << "]: "
      << (*this) << std::endl;
    }
  }

  void start(const char* msg = 0);
  void restart(const char* msg = 0);
  void resume(const char* msg = 0);
  void stop(const char* msg = 0);
  void check(const char* msg = 0);

  double measured_time() const;
}
; // class Timer

//===========================================================================
// Return the total time that the timer has been in the "running"
// state since it was first "started" or last "restarted".  For
// "short" time periods (less than an hour), the actual cpu time
// used is reported instead of the elapsed time.

inline double Timer::elapsed_time() const {
  time_t acc_sec = time(0) - start_time;
  if (acc_sec < 3600)
    return (clock() - start_clock) / (1.0 * CLOCKS_PER_SEC);
  else
    return (1.0 * acc_sec);

} // Timer::elapsed_time

//===========================================================================
// Start a timer.  If it is already running, let it continue running.
// Print an optional message.

inline void Timer::start(const char* msg) {
// Print an optional message, something like "Starting timer t";
  if (msg) std::cout << msg << std::endl;

// Return immediately if the timer is already running
  if (running) return;

// Set timer status to running and set the start time
  running = true;
  start_clock = clock();
  start_time = time(0);

} // Timer::start

//===========================================================================
// Turn the timer off and start it again from 0.  Print an optional message.

inline void Timer::restart(const char* msg) {
// Print an optional message, something like "Restarting timer t";
  if (msg) std::cout << msg << std::endl;

// Set timer status to running, reset accumulated time, and set start time
  running = true;
  acc_time = 0;
  start_clock = clock();
  start_time = time(0);

} // Timer::restart

//===========================================================================
// Turn the timer on again.  Print an optional message.

inline void Timer::resume(const char* msg) {
// Print an optional message, something like "Restarting timer t";
  if (msg) std::cout << msg << std::endl;

// Return immediately if the timer is already running
  if (running) return;

  // Set timer status to running, reset accumulated time, and set start time
  running = true;
  start_clock = clock();
  start_time = time(0);

} // Timer::restart

//===========================================================================
// Stop the timer and print an optional message.

inline void Timer::stop(const char* msg) {
// Print an optional message, something like "Stopping timer t";
  if (msg) std::cout << msg << std::endl;

// Compute accumulated running time and set timer status to not running
  if (running) acc_time += elapsed_time();
  running = false;

} // Timer::stop

//===========================================================================
// Print out an optional message followed by the current timer timing.

inline void Timer::check(const char* msg) {
// Print an optional message, something like "Checking timer t";
  if (msg) std::cout << msg << " : ";

  std::cout << "Elapsed time [" << std::setiosflags(std::ios::fixed )
  << std::setprecision(2)
  << acc_time + (running ? elapsed_time() : 0) << "] seconds\n";

} // Timer::check

//===========================================================================
// Allow timers to be printed to ostreams using the syntax 'os << t'
// for an ostream 'os' and a timer 't'.  For example, "cout << t" will
// print out the total amount of time 't' has been "running".

inline std::ostream& operator<<(std::ostream& os, const Timer& t) {
  os << std::setprecision(2) << std::setiosflags(std::ios::fixed )
  << t.acc_time + (t.running ? t.elapsed_time() : 0);
  return os;
}

//===========================================================================


inline double Timer::measured_time() const {
  return acc_time + (running ? elapsed_time() : 0);
}
#endif // TIMER_H

