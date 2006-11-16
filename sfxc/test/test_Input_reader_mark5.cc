#include <Input_reader_mark5.h>
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

  Input_reader_mark5 reader;
  std::ofstream out(argv[1], std::ios::out | std::ios::binary);

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

  int nGet, nForward;
  while ((nGet = reader.get_bytes(BUFFSIZE, buff)) > 0) {
    out.write(buff, nGet*sizeof(char));
    nForward = reader.move_forward(nGet);
    assert (nGet == nForward);
    usleep(10);
  }

  out.close();

  return 0;
}
