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

//c++ includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;


#include "Log_writer.h"


class Timer {

  public:
    /** Start a timer **/
    void tmrBegin(Log_writer &log_writer);

    /** Stop a timer **/
    void tmrEnd(Log_writer &log_writer);

  private:
    clock_t  CPUbegin;
    clock_t  CPUend;
    time_t   Tbegin;
    time_t   Tend;
    string   ID[100];

};

#endif