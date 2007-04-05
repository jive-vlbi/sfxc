/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Tests reading data from a Mark5 (simulation using File2net).
*/

#include <types.h>
#include <Data_reader_mark5.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>
#include <unistd.h>

#define BUFFSIZE 1000

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <output_file>" << std::endl;
    exit(1);
  }
  int nBytes;
  char buff[BUFFSIZE];

  Data_reader_mark5 reader("tcp", 2630);
  std::ofstream out(argv[1], std::ios::out | std::ios::binary);

  nBytes= reader.get_bytes(1, buff);
  assert (nBytes == 1);
  out.write(buff, nBytes*sizeof(char));

  nBytes= reader.get_bytes(10, buff);
  assert (nBytes == 10);
  out.write(buff, nBytes*sizeof(char));

  nBytes= reader.get_bytes(100, buff);
  assert (nBytes == 100);
  out.write(buff, nBytes*sizeof(char));

  for (int i=0; i<10; i++) {
    nBytes = reader.get_bytes(50, buff);
    assert(nBytes==50);
    out.write(buff, nBytes*sizeof(char));
  }

  nBytes = reader.get_bytes(BUFFSIZE, buff);
  assert(nBytes==BUFFSIZE);
  out.write(buff, nBytes*sizeof(char));

  int nGet;
  while ((nGet = reader.get_bytes(BUFFSIZE, buff)) > 0) {
    out.write(buff, nGet*sizeof(char));
  }

  out.close();

  return 0;
}
