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

#include <unistd.h>
#include <sys/time.h>

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

#define INVALID_TIME              -1
#define MAX_SAMPLE_RATE  (64000000)
/// Constants
// Some constants define the headers in the input -> correlator_node steam
#define HEADER_DATA       0
#define HEADER_INVALID    1
#define HEADER_DELAY      2
#define HEADER_ENDSTREAM  3

// The Fillpattern inserted by the streamstore card
#define MARK5_FILLPATTERN (0x11223344)
// The minimum number of words of "fillpattern" inserted by the streamstore card
#define MARK5_FILLPATTERN_NWORDS   (65528/4)

// The (maximum) amount of samples processed per iteration, this is automatically set to a multiple of nchannels
#define CORRELATOR_BUFFER_SIZE    8192

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

#define SIZE_VDIF_WORD            sizeof(int32_t)

// Number of mark5b frames to read at once
// To make sure that the first sample lies exactly on an integer microsecond
// duration of one block = SIZE_MK5B_WORD * SIZE_MK5B_FRAME * 8 / (MAX_SAMPLE_RATE * MAX_N_BITSTREAMS)
//                       = 4 * 2500 * 8 / (64 * 32) = 625 / 16
// MAX_SAMPLE_RATE = 64 MHz
// Hence, this should be a multiple of 16
#define N_MK5B_BLOCKS_TO_READ      16

#define SFXC_PORT                1233
#define MAX_TCP_CONNECTIONS      16

#define SFXC_INVALIDATE_SAMPLES
#ifdef SFXC_INVALIDATE_SAMPLES
//#  define SFXC_CHECK_INVALID_SAMPLES
#  ifdef SFXC_CHECK_INVALID_SAMPLES
// Used to fill the random data for testing. Should be char(0) or char(-1)
#    define INVALID_PATTERN          char(0)
#  endif
#endif

#define RED(str) "[\033[31m"+str+"\033[30m]"
#define GREEN(str) "[\033[32m"+str+"\033[30m]"
#define YELLOW(str) "[\033[33m"+str+"\033[30m]"
#define CYAN(str) "[\033[36m"+str+"\033[30m]"

/// This is the MPI rank or the processID
extern int RANK_OF_NODE;
// Rank of the current node

#ifdef SFXC_PRINT_DEBUG
#define FORMAT_MSG(msg) \
    "#" << RANK_OF_NODE << " " \
    << __FILE__ << ", " << __LINE__ << ": " \
    << msg

#ifdef USE_MPI
#include <mpi.h>
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

void abort_sfxc_assertion(const char *file, int line, const char* message);
void sfxc_abort(const char *msg="");

#ifdef NDEBUG
#define SFXC_ASSERT(c)
#define SFXC_ASSERT_MSG(c, msg)
#else
#define SFXC_ASSERT(c) \
  { if (!(c)) abort_sfxc_assertion(__FILE__, __LINE__, #c ); }

#define SFXC_ASSERT_MSG(c, msg) \
  { if (!(c)) abort_sfxc_assertion(__FILE__, __LINE__, msg ); }
#endif // NDEBUG

#ifdef USE_IPP
  #ifdef USE_DOUBLE
    #define FLOAT                   double
    #define SFXC_ZERO_F             sfxc_zero
    #define SFXC_ZERO_FC            sfxc_zero_c
    #define SFXC_FFT                sfxc_fft_ipp
    #define SFXC_MUL_FC_I           sfxc_mul_c_I
    #define SFXC_CONJ_FC            sfxc_conj_c
    #define SFXC_ADD_PRODUCT_FC     sfxc_add_product_c 
    #define SFXC_MUL_F              sfxc_mul
  #else // !USE_DOUBLE
    #define FLOAT                   float
    #define SFXC_ZERO_F             sfxc_zero_f
    #define SFXC_ZERO_FC            sfxc_zero_fc
    #define SFXC_FFT                sfxc_fft_ipp_float
    #define SFXC_MUL_FC_I           sfxc_mul_fc_I
    #define SFXC_CONJ_FC            sfxc_conj_fc
    #define SFXC_ADD_PRODUCT_FC     sfxc_add_product_fc 
    #define SFXC_MUL_F              sfxc_mul_f
  #endif
  #define SFXC_FFT_FLOAT          sfxc_fft_ipp_float
#else
  #include <fftw3.h>
  #ifdef USE_DOUBLE
    #define FLOAT                   double
    #define FFTW_COMPLEX            fftw_complex
    #define SFXC_ZERO_F             sfxc_zero
    #define SFXC_ZERO_FC            sfxc_zero_c
    #define SFXC_FFT                sfxc_fft_fftw
    #define SFXC_MUL_FC_I           sfxc_mul_c_I
    #define SFXC_CONJ_FC            sfxc_conj_c
    #define SFXC_ADD_PRODUCT_FC     sfxc_add_product_c 
    #define SFXC_MUL_F              sfxc_mul
  #else // !USE_DOUBLE
    #define FLOAT                   float
    #define FFTW_COMPLEX            fftwf_complex
    #define SFXC_ZERO_F             sfxc_zero_f
    #define SFXC_ZERO_FC            sfxc_zero_fc
    #define SFXC_FFT                sfxc_fft_fftw_float
    #define SFXC_MUL_FC_I           sfxc_mul_fc_I
    #define SFXC_CONJ_FC            sfxc_conj_fc
    #define SFXC_ADD_PRODUCT_FC     sfxc_add_product_fc 
    #define SFXC_MUL_F              sfxc_mul_f
  #endif
  #define SFXC_FFT_FLOAT          sfxc_fft_fftw_float
#endif

// Supported window functions
#define SFXC_WINDOW_NONE       0   // disable windowing
#define SFXC_WINDOW_RECT       1   // rectangular window
#define SFXC_WINDOW_COS        2   // Cosine window
#define SFXC_WINDOW_HAMMING    3   // Hamming window
#define SFXC_WINDOW_HANN       4   // Hann window

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
 * Report is integer is a power of two
 **/
bool isPower2(int val);

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

inline double toMB(unsigned long long size) {
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
