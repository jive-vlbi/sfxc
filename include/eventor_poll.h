/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *  This file contains:
 *     - An event generator based on the poll function
 */
#ifndef EVENTOR_POLL_H_INCLUDED
#define EVENTOR_POLL_H_INCLUDED

#include <poll.h>
#include <vector>
#include "exception_common.h"
#include "utils.h"
#include "timer.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

class FdEventListener {
public:
  /// The function that is called if an event is detected.
  virtual void on_event(short events) = 0;
  virtual void on_error(short events) = 0;
};

class Eventor_poll {
  std::vector<FdEventListener*> listeners_;
  std::vector<struct pollfd> pollif_;

  /// An array storing in which order the listeners_
  /// have to be treated.
  std::vector<int> shuffler_;

  /// Time spend in waiting for data to arrive
  Timer timer_breading_;

  /// Time spend in really reading the data
  Timer timer_reading_;

public:
  Eventor_poll() {
  }

  ~Eventor_poll() {

    double total_duration=timer_breading_.measured_time()+timer_reading_.measured_time();
    double ratio1 = ((100.0*timer_breading_.measured_time())/total_duration);
    double ratio2 = ((100.0*timer_reading_.measured_time())/total_duration);
    PROGRESS_MSG( "enventor_poll: ratio:("<< ratio1 <<"%, "<< ratio2 <<"%)");
  }

  void add_listener( short events, int fd, FdEventListener* listener ) {
    struct pollfd pollfd;
    pollfd.fd = fd;
    pollfd.events = events;

    shuffler_.push_back( listeners_.size() );

    listeners_.push_back(listener);
    pollif_.push_back( pollfd );
  }


  void randomize() {
    for (unsigned int i=0;i<shuffler_.size();i++) {
      int a = ((1.0*random()/RAND_MAX)*shuffler_.size());
      int b = ((1.0*random()/RAND_MAX)*shuffler_.size());
      std::swap( shuffler_[a], shuffler_[b] );
    }
  }


  void wait_until_any_event() {
    /// waiting something happens on the descriptor we are supposed
    /// to monitor -1 == no timeout.
    timer_breading_.resume();
    int ret = poll( &(pollif_[0]), pollif_.size(), -1);
    timer_breading_.stop();

    /// it cannot be zero because the timeout is set to infinite
    SFXC_ASSERT( ret != 0 );

    /// If the returned value == 0.
    timer_reading_.resume();
    if (ret > 0) {
      /// Scan all the entries.
      for (unsigned int i=0;i< pollif_.size() && ret>0;i++) {
        int idx = shuffler_[i];
        /// Check if there is an event on this descriptor
        if ( pollif_[idx].revents != 0 ) {
          /// check if there is an error
          if ( pollif_[idx].revents & (POLLERR | POLLHUP| POLLNVAL) ) {
            listeners_[idx]->on_error( pollif_[idx].revents );
          }
          /// Check if there is a normal event
          if ( pollif_[idx].revents & (POLLIN | POLLPRI | POLLOUT) ) {
            listeners_[idx]->on_event( pollif_[idx].revents );
          }
        }
      }
    } else {
      MTHROW("An exception occurs during poll()");
    }
    timer_reading_.stop();

  }


#ifdef ENABLE_TEST_UNIT
class Test : public Test_aclass< Eventor_poll > {
  class MyListener : public FdEventListener {
    public:
      void on_event(short events) {
        std::cout << "Something happens on the file descriptor" << std::endl;
      }
      void on_error(short events) {
        std::cout << "An error happens on the file descriptor" << std::endl;
      };
    };

  public:
    void tests();
  };
#endif //ENABLE_TEST_UNIT
};


#ifdef ENABLE_TEST_UNIT
void Eventor_poll::Test::tests() {
  Eventor_poll helper;
  MyListener listener;

  helper.add_listener(POLLIN | POLLPRI, fileno( stdin ), &listener );
  helper.wait_until_any_event();
}
#endif // ENABLE_TEST_UNIT


#endif // EVENTOR_POLL_H_INCLUDED
