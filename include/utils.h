/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include <types.h>
#include <list>
#include <string>
#include "Log_writer.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"


#ifdef SFXC_PRINT_DEBUG
#define DEBUG_MSG(msg) \
    std::cout << __FILE__ << "," << __LINE__ << ": " << msg << std::endl; 
#else
#define DEBUG_MSG(msg) 
#endif

/// Interface_pair is a pair of two strings 
/// containing the interface name and its ip address;
typedef std::pair<std::string, std::string> Interface_pair;

/** 
 * get_ip_address retrieves the ip-addresses of the current host.
 * @returns a list of Interface_pair.
 **/ 
void get_ip_address(std::list<Interface_pair> &addresses,
                    bool IPv4_only = true);
          
/** 
 * get the time in micro seconds from an array (year, day, hour, minute, second).
 * @returns a list of Interface_pair.
 **/ 
INT64 get_us_time(int time[]);


/** Initialises the global control files, this should be removed at some point.
  **/
int initialise_control(const char *filename, Log_writer &log_writer, 
  RunP &RunPrms, GenP &GenPrms, StaP StaPrms[]);
                    
///** Constructs an array containing all control data for a correlate node (Gen and Run).
// *  @returns the size of the array
//  **/
//void send_control_data(int rank);
//
///** Stores all control data for a correlate node (Gen and Run).
//  **/
//void receive_control_data(MPI_Status &status);
                    
#endif // UTILS_H
