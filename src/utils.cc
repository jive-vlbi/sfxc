/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 */

#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <assert.h>


#include <genFunctions.h>
#include <iostream>
using namespace std;
//constants
#include "constPrms.h"

//class and function definitions
#include "genFunctions.h"
#include "InData.h"

#include <Data_reader_file.h>

#define SEED 10


// used for randomising numbers for Headers in Mk4 file
extern UINT32 seed;

INT64 get_us_time(int time[]) {
  INT64 result = 0;
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


/** Initialises the global control files, this should be removed at some point.
 **/
int
initialise_control(char *filename, Log_writer &log_writer, 
  RunP &RunPrms, GenP &GenPrms, StaP StaPrms[]) 
{
  int    i, Nstations;
  
  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(filename, log_writer) != 0) {
    cerr << "ERROR: Control file "<< filename <<", program aborted.\n";
    return -1;
  }

  //show version information and control file info
  if (RunPrms.get_messagelvl() > 0)
    log_writer << "Source " << __FILE__ << " compiled at: "
         << __DATE__ << " " <<__TIME__ << "\n"
         << "Control file name "  <<  filename << "\n";
  
  //check control parameters, optionally show them
  if (RunPrms.check_params(log_writer) != 0) {
    log_writer << "ERROR: Run control parameter, program aborted.\n";
    return -1;
  }
  
  log_writer.ask_continue();

  //parse control file for general parameters
  if (GenPrms.parse_ctrlFile(filename,log_writer) != 0) {
    log_writer << "ERROR: Control file "<< filename <<", program aborted.\n";
    return -1;
  }

  //check general control parameters, optionally show them
  if (GenPrms.check_params(log_writer) != 0) {
    cerr << "ERROR: General control parameter, program aborted.\n";
    return -1;
  }
  
  log_writer.ask_continue();

  //get the number of stations
  Nstations = GenPrms.get_nstations();
  
  //parse the control file for all station parameters
  for (i=0; i<Nstations; i++)
    if (StaPrms[i].parse_ctrlFile(filename,i, log_writer) != 0 ) {
      log_writer << "ERROR: Control file "<< filename <<", program aborted.\n";
      return -1;
    }
    
  //check station control parameters, optionally show them
  for (i=0; i<Nstations; i++){
    if (StaPrms[i].check_params(log_writer) != 0 ) {
      log_writer << "ERROR: Station control parameter, program aborted.\n";
      return -1;
    }
    log_writer.ask_continue();
  }
  
  return 0;  
}
