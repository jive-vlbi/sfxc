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
#include <assert.h>
#include "Log_writer.h"

#include <sfxc_mpi.h>

/// Constants
#define SIZE_MK4_FRAME           20000
// Maximal track bit rate
#define MAX_MARK4_TRACK_BIT_RATE 16
// Maximal delay in milliseconds, should be an integer number of Mark4 blocks
// 5 should be enough
#define MAX_DELAY                5
// The amount of padding in the correlator (could be 1, not tested though)
#define PADDING                  2

// NGHK: remove?
const int   BufTime       =   16384; //delta time for Bufs in micro seconds


#define RED(str) "[\033[31m"+str+"\033[30m]"
#define GREEN(str) "[\033[32m"+str+"\033[30m]"
#define YELLOW(str) "[\033[33m"+str+"\033[30m]"
#define CYAN(str) "[\033[36m"+str+"\033[30m]"

#ifdef SFXC_PRINT_DEBUG
extern int RANK_OF_NODE; // Rank of the current node
//#define FORMAT_MSG(msg) \
//    "\033[32m#" << RANK_OF_NODE << " " \
//    << __PRETTY_FUNCTION__ << "," << __LINE__ << "\033[30m: " \
//    << msg
#define FORMAT_MSG(msg) \
    "\033[32m#" << RANK_OF_NODE << " " \
    << __FILE__ << ", " << __LINE__ << "\033[30m: " \
    << msg
#define DEBUG_MSG(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      std::cout << FORMAT_MSG(msg) << std::endl << std::flush; }
#define MPI_DEBUG_MSG(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      get_log_writer()(0) << FORMAT_MSG(msg) << std::endl << std::flush; }
#else
#define DEBUG_MSG(msg) 
#define MPI_DEBUG_MSG(msg) 
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
int64_t get_us_time(int time[]);


///** Constructs an array containing all control data for a correlate node (Gen and Run).
// *  @returns the size of the array
//  **/
//void send_control_data(int rank);
//
///** Stores all control data for a correlate node (Gen and Run).
//  **/
//void receive_control_data(MPI_Status &status);


//*****************************************************************************
//  irbit2: random seeding
//  See Numerical Recipes
//  primitive polynomial mod 2 of order n produces 2^n - 1 random bits
//*****************************************************************************
void set_seed(unsigned long seed_);
int irbit2();

                    
#endif // UTILS_H
