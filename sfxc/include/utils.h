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

#ifndef PRINT_PID
#define PRINT_PID         true
#endif // PRINT_PID

#ifndef PRINT_HOST
#define PRINT_HOST        false
#endif // PRINT_HOST

#ifndef PRINT_TIMER
#define PRINT_TIMER       false
#endif // PRINT_TIMER

/// Constants
#define SIZE_VLBA_FRAME           20000
#define SIZE_VLBA_HEADER          96
#define SIZE_VLBA_AUX_HEADER      64

#define SIZE_MK5A_FRAME           20000
// Size of a mark5a header
#define SIZE_MK5A_HEADER          160

// Size of a Mark5B frame
#define SIZE_MK5B_FRAME           2500
#define SIZE_MK5B_HEADER             4
#define SIZE_MK5B_WORD            sizeof(int32_t)
// Number of mark5b frames to read at once
// To make sure that the first sample lies exactly on an integer microsecond
// duration of one block: SIZE_MK5B_FRAME/MAX_SAMPLE_RATE = 2500/64 = 625/16
// MAX_SAMPLE_RATE = 64 MHz
// Hence, this should be a multiple of 16
#define N_MK5B_BLOCKS_TO_READ       16

// The amount of padding in the correlator (could be 1, not tested though)
#define PADDING                  2

#define SFXC_PORT                1233
#define MAX_TCP_CONNECTIONS      16

#define SFXC_INVALIDATE_SAMPLES
#ifdef SFXC_INVALIDATE_SAMPLES
//#  define SFXC_CHECK_INVALID_SAMPLES
#  ifdef SFXC_CHECK_INVALID_SAMPLES
// Used to fill the random data for testing. Should be char(0) or char(-1)
#    define INVALID_PATTERN          char(-1)
#  endif
#endif

#define RED(str) "[\033[31m"+str+"\033[30m]"
#define GREEN(str) "[\033[32m"+str+"\033[30m]"
#define YELLOW(str) "[\033[33m"+str+"\033[30m]"
#define CYAN(str) "[\033[36m"+str+"\033[30m]"

/// This is the MPI rank or the processID
extern int RANK_OF_NODE;
// Rank of the current node

#ifdef USE_MPI
#include "sfxc_mpi.h"
#endif //USE_MPI


#ifdef SFXC_PRINT_DEBUG
#define FORMAT_MSG(msg) \
    "#" << RANK_OF_NODE << " " \
    << __FILE__ << ", " << __LINE__ << ": " \
    << msg

#ifdef USE_MPI
#define DEBUG_MSG(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      std::cout << FORMAT_MSG(msg) << std::endl << std::flush; }
#define DEBUG_MSG_RANK(rank, msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      if (RANK_OF_NODE == rank) { std::cout << FORMAT_MSG(msg) << std::endl << std::flush; } }
#define DEBUG_MSG_MPI(msg) \
    { if (RANK_OF_NODE < 0) { MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE); }; \
      get_log_writer()(0) << FORMAT_MSG(msg) << std::endl << std::flush; }
#else // USE_MPI
#define DEBUG_MSG(msg) \
    { std::cout << FORMAT_MSG(msg) << std::endl << std::flush; }
#define DEBUG_MSG_RANK(rank, msg) \
    { if (RANK_OF_NODE == rank) { std::cout << FORMAT_MSG(msg) << std::endl << std::flush; } }
#define DEBUG_MSG_MPI(msg) \
    { get_log_writer()(0) << FORMAT_MSG(msg) << std::endl << std::flush; }
#endif // USE_MPI

#else
#define DEBUG_MSG(msg)
#define DEBUG_MSG_RANK(rank,msg)
#define MPI_DEBUG_MSG(msg)
#endif

void abort_sfxc(const char *file, int line, const char* message);
void abort(const char *file, int line, const char* message);

#ifdef NDEBUG
#define SFXC_ASSERT(c)
#define SFXC_ASSERT_MSG(c, msg)
#else
#ifdef USE_MPI
#define SFXC_ASSERT(c) \
  { if (!(c)) abort_sfxc(__FILE__, __LINE__, #c ); }

#define SFXC_ASSERT_MSG(c, msg) \
  { if (!(c)) abort_sfxc(__FILE__, __LINE__, msg ); }
#else
#define SFXC_ASSERT(c) \
  { if (!(c)) abort(__FILE__, __LINE__, #c ); }

#define SFXC_ASSERT_MSG(c, msg) \
  { if (!(c)) abort(__FILE__, __LINE__, msg ); }
#endif // USE_MPI
#endif // NDEBUG

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

bool directory_exist(const std::string& path);
void create_directory(const std::string& path);

// Modified julian day
int mjd(int day, int month, int year);

inline double toMB(unsigned int size) {
  return (1.0*size)/(1024*1024);
}

std::string itoa (int32_t n);

#ifndef MIN
#define MIN(x, y) (((x)<(y))?(x):(y))
#endif // MIN

#ifndef MAX
#define MAX(x, y) (((x)>(y))?(x):(y))
#endif // MAX


#endif // UTILS_H
