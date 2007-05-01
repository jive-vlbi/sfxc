/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * timer structure and prototypes
 */

#ifndef TIMER_H
#define TIMER_H

//c includes
#include <time.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;


#include "Log_writer.h"


class Timer {

  public:

    /** Sets default values for:
        CPU_elapsed = CPU_accu = 0.0;
        T_elapsed = T_accu = 0.0;
        ID = "unknown process ";
    **/
    Timer();

    /** Start a timer **/
    void start(Log_writer &log_writer);
    void start();

    /** Stop a timer **/
    void stop(Log_writer &log_writer);

    /** Stop timer and accumulate timer results **/
    void stop_accumulate();

    /** Show accumulated results **/
    void show_accu_result(Log_writer &log_writer);

    /** Set a timer ID**/
    void set_ID(string ID_)
    {
      ID = ID_;
    }

  private:
    clock_t  CPUbegin;
    clock_t  CPUend;
    time_t   Tbegin;
    time_t   Tend;
    string   ID;
    double   CPU_elapsed, CPU_accu;
    double   T_elapsed, T_accu;

};

#endif
