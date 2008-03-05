/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - sfxc/SCARIe
 * This file contains:
 *   - the main benchmarking application.
 */
#define VERBOSE

#include <iostream>
#include <vector>
#include "benchmark.h"
#include "channel_extractor_interface.h"
#include "channel_extractor_brute_force.h"
#include "channel_extractor_5.h"
#include "channel_extractor_fast.h"



int main() {
  std::vector<Benchmark*> benchmarks;
  benchmarks.push_back( new Benchmark( *(new Channel_extractor_brute_force()) ) );
  //benchmarks.push_back( new Benchmark( *(new Channel_extractor_5()) ) );
  //benchmarks.push_back( new Benchmark( *(new Channel_extractor_fast()) ) );
  //benchmarks.push_back( new Benchmark( *(new Channel_extractor_dynamic()) ) );

  for (unsigned int i=0;i<benchmarks.size();i++) {
    std::cout << "Testing: " << benchmarks[i]->get_extractor().name() <<  "    ";
    std::cout.flush();
    if ( benchmarks[i]->test() ) {
      std::cout << " [PASSED] \n";
    } else {
      std::cout << " [FAILED] \n";
    }
  }
  /*
  // Benchmarking is not yet working...please wait...
  for(unsigned int i=0;i<benchmarks.size();i++){
     std::cout << "Benchmarking: " << benchmarks[i]->get_extractor().name() <<  "    ";
     std::cout.flush();
     benchmarks[i]->benchmark(1024, 3000);
     std::cout << std::endl;
   }
  */
}
