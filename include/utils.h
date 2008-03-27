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

#include <list>
#include <string>
#include <assert.h>

# include <unistd.h>
# include <sys/time.h>

#include "types.h"
#include "log_writer.h"

#define PRINT_PID         true
#define PRINT_HOST        false
#define PRINT_TIMER       false

/// Constants
#define SIZE_MK4_FRAME           20000
// Size of a mark4 header
#define SIZE_MK4_HEADER          160
// Track bit rate
#define MARK4_TRACK_BIT_RATE     8000000
// Maximal delay in milliseconds, should be an integer number of Mark4 blocks
// 5 should be enough
#define MAX_DELAY                5
// The amount of padding in the correlator (could be 1, not tested though)
#define PADDING                  2

#define SFXC_PORT                1233
#define MAX_TCP_CONNECTIONS      16


// NGHK: remove?
const int   BufTime       =   16384; //delta time for Bufs in micro seconds


#define RED(str) "[\033[31m"+str+"\033[30m]"
#define GREEN(str) "[\033[32m"+str+"\033[30m]"
#define YELLOW(str) "[\033[33m"+str+"\033[30m]"
#define CYAN(str) "[\033[36m"+str+"\033[30m]"

#ifdef SFXC_PRINT_DEBUG
#include "sfxc_mpi.h"

extern int RANK_OF_NODE; // Rank of the current node
/*
#define FORMAT_MSG(msg) \
    "\033[32m#" << RANK_OF_NODE << " " \
    << __PRETTY_FUNCTION__ << "," << __LINE__ << "\033[30m: " \
    << msg
*/
#define FORMAT_MSG(msg) \
    "#" << RANK_OF_NODE << " " \
    << __FILE__ << ", " << __LINE__ << ": " \
    << msg
#define DEBUG_MSG(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      std::cout << FORMAT_MSG(msg) << std::endl << std::flush; }
#define DEBUG_MSG_RANK(rank, msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      if (RANK_OF_NODE == rank) { std::cout << FORMAT_MSG(msg) << std::endl << std::flush; } }
#define DEBUG_MSG_MPI(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      get_log_writer()(0) << FORMAT_MSG(msg) << std::endl << std::flush; }
#else
#define DEBUG_MSG(msg)
#define DEBUG_MSG_RANK(rank,msg)
#define MPI_DEBUG_MSG(msg)
#endif


#define USE_DOUBLE

#ifdef USE_DOUBLE
# define FLOAT                   double
# define FFTW_COMPLEX            fftw_complex
# define FFTW_PLAN               fftw_plan
# define FFTW_PLAN_DFT_1D        fftw_plan_dft_1d
# define FFTW_PLAN_DFT_R2C_1D    fftw_plan_dft_r2c_1d
# define FFTW_EXECUTE            fftw_execute
# define FFTW_EXECUTE_DFT        fftw_execute_dft
# define FFTW_EXECUTE_DFT_R2C    fftw_execute_dft_r2c
# define FFTW_DESTROY_PLAN       fftw_destroy_plan
#else // !USE_DOUBLE
# define FLOAT                   float
# define FFTW_COMPLEX            fftwf_complex
# define FFTW_PLAN               fftwf_plan
# define FFTW_PLAN_DFT_1D        fftwf_plan_dft_1d
# define FFTW_PLAN_DFT_R2C_1D    fftwf_plan_dft_r2c_1d
# define FFTW_EXECUTE            fftwf_execute
# define FFTW_EXECUTE_DFT        fftwf_execute_dft
# define FFTW_EXECUTE_DFT_R2C    fftwf_execute_dft_r2c
# define FFTW_DESTROY_PLAN       fftwf_destroy_plan
#endif // USE_FLOAT


#ifdef PRINT_PROGRESS
inline void getusec(unsigned long long &utime) {
  struct timeval tv;
  gettimeofday(&tv,0);
  utime=tv.tv_sec*1000000;
  utime+=tv.tv_usec;
}

inline unsigned long long getusec(void) {
  unsigned long long t;
  getusec(t);
  return t;
}

#define PROGRESS_MSG(msg) DEBUG_MSG(" PROGRESS t=" << getusec() << ": " << msg);
#else
#define PROGRESS_MSG(msg)
#endif // PRINT_PROGRESS

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

//*****************************************************************************
//  irbit2: random seeding
//  See Numerical Recipes
//  primitive polynomial mod 2 of order n produces 2^n - 1 random bits
//*****************************************************************************
void set_seed(unsigned long seed_);
int irbit2();

// Park-Miller random number generator for 31-bit random numbers
void park_miller_set_seed(unsigned long seed_);
long unsigned int park_miller_random();


#endif // UTILS_H
