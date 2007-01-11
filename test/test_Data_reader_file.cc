/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
  
  Author: Nico Kruithof <Kruithof@jive.nl>
  Checks reading data from file using a Data_reader.
*/


#include <Data_reader_file.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

char *infile = "data/input.txt";
char *outfile = "output.txt";
#define BUFFSIZE 1000

int main(int argc, char *argv[]) {
  int nBytes;
  char buff[BUFFSIZE];

  Data_reader_file reader(infile);
  std::ofstream out(outfile, std::ios::out | std::ios::binary);

  nBytes= reader.get_bytes(100, buff);
  assert(nBytes==100);
  out.write(buff, nBytes*sizeof(char));

  nBytes = reader.get_bytes(250, buff);
  assert(nBytes==250);
  out.write(buff, nBytes*sizeof(char));

  for (int i=0; i<10; i++) {
    nBytes = reader.get_bytes(50, buff);
    assert(nBytes==50);
    out.write(buff, nBytes*sizeof(char));
  }

  nBytes = reader.get_bytes(BUFFSIZE, buff);
  assert(nBytes==BUFFSIZE);
  out.write(buff, nBytes*sizeof(char));

  bool lastBlock = false;
  while ((nBytes = reader.get_bytes(BUFFSIZE, buff)) > 0) {
    assert(!lastBlock); 
    if (nBytes != BUFFSIZE) lastBlock = true; // Only last block can have smaller size
    out.write(buff, nBytes*sizeof(char));
  }

  out.close();
  
  //check output:
  std::string command = "diff ";
  command += infile; command += " "; command += outfile;
  int result = system(command.c_str());

  if (result != 0) {
    std::cout << "ERROR: Difference in files" << std::endl;
    exit(1);
  }
  
  //remove(outfile);

  return 0;
}
