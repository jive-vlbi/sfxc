/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <assert.h>
#include <iostream>

#include "utils.h"
#include <data_reader_file.h>

#ifdef SFXC_PRINT_DEBUG
int RANK_OF_NODE = -1; // Rank of the current node
#endif

int64_t get_us_time(int time[]) {
  int64_t result = 0;
  // time[0] is year
  result = time[2]; // hours
  result = time[3] +   60* result; // minutes
  result = time[4] +   60* result; // seconds
  result = 0       + 1000* result; // milisecs
  result = 0       + 1000* result; // microsecs
  
  return result;
}

void get_ip_address(std::list<Interface_pair> &addresses,
                    bool IPv4_only) {
  struct ifaddrs *ifa = NULL;
  
  if (getifaddrs (&ifa) < 0) {
    perror ("getifaddrs");
    return;
  }

  for (; ifa; ifa = ifa->ifa_next) {
    char ip[ 200 ];
    socklen_t salen;

    if (ifa->ifa_addr->sa_family == AF_INET) {
      // IP v4
      salen = sizeof (struct sockaddr_in);
    } else if (!IPv4_only && (ifa->ifa_addr->sa_family == AF_INET6)) {
      // IP v6
      salen = sizeof (struct sockaddr_in6);
    } else {
      continue;
    }

    if (getnameinfo (ifa->ifa_addr, salen,
                     ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0) {
      perror ("getnameinfo");
      continue;
    }
    addresses.push_back(Interface_pair(std::string(ifa->ifa_name),
                                       std::string(ip)));
  }

  freeifaddrs (ifa);
}

//*****************************************************************************
//  irbit2: random seeding
//  See Numerical Recipes
//  primitive polynomial mod 2 of order n produces 2^n - 1 random bits
//*****************************************************************************
unsigned long iseed = 42;
void set_seed_1_bit(unsigned long seed_) {
  assert(seed_ != 0);
  iseed = seed_;
}

int irbit2()
{
  #define IB1 1
  //  #define IB2 2
  #define IB4 8
  //  #define IB5 16
  #define IB6 32 
  //  #define IB18 131072
  #define IB30 536870912
  #define MASK (IB1+IB4+IB6)
 
  if (iseed & IB30) {
    iseed=((iseed ^ MASK) << 1) | IB1;
    return 1;
  } else {
    iseed <<= 1;
    return 0;
  }
  #undef MASK
  #undef IB30
  //  #undef IB18
  #undef IB6
  //  #undef IB5
  #undef IB4
  //  #undef IB2
  #undef IB1
}

long park_miller_seed = 42;
void park_miller_set_seed(unsigned long seed_) {
  assert(seed_ != 0);
  park_miller_seed = seed_;
}

// Generated 31 random bits at a time
long unsigned int park_miller_random() {
  long unsigned int hi, lo;

  lo = 16807 * (park_miller_seed & 0xFFFF);
  hi = 16807 * (park_miller_seed >> 16);

  lo += (hi & 0x7FFF) << 16;
  lo += hi >> 15;

  if (lo > 0x7FFFFFFF) lo -= 0x7FFFFFFF;

  return (park_miller_seed = (long)lo);
}