#include <Input_reader_file.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

char *infile = "data/input.txt";
char *outfile = "output.txt";
#define BUFFSIZE 1000

int main(int argc, char *argv[]) {
  int nBytes, nForward;
  char buff[BUFFSIZE];

  Input_reader_file reader(infile);
  std::ofstream out(outfile, std::ios::out | std::ios::binary);

  nBytes= reader.get_bytes(100, buff);
  assert(nBytes==100);
  out.write(buff, nBytes*sizeof(char));
  nBytes = reader.move_forward(100);
  assert (nBytes == 100);

  nBytes = reader.get_bytes(250, buff);
  assert(nBytes==250);
  out.write(buff, nBytes*sizeof(char));
  nBytes = reader.move_forward(250);
  assert (nBytes == 250);

  for (int i=0; i<10; i++) {
    nBytes = reader.get_bytes(50, buff);
    assert(nBytes==50);
    out.write(buff, nBytes*sizeof(char));
    nBytes = reader.move_forward(50);
    assert (nBytes == 50);
  }

  nBytes = reader.get_bytes(BUFFSIZE, buff);
  assert(nBytes==BUFFSIZE);
  out.write(buff, nBytes*sizeof(char));
  nBytes = reader.move_forward(BUFFSIZE);
  assert (nBytes == BUFFSIZE);

  while ((nBytes = reader.get_bytes(BUFFSIZE, buff)) > 0) {
    //std::cout << nBytes << std::endl;
    out.write(buff, nBytes*sizeof(char));
    nForward = reader.move_forward(nBytes);
    assert (nBytes == nForward);
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
