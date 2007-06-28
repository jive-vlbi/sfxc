/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Checks reading data from file using a Data_reader.
*/


#include <Data_reader_file.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

char *infile = "data/input.txt";
char *outfile = "output.txt";
#define BUFFSIZE 1000

#include <Semaphore_buffer.h>
#include <Data_writer_file.h>
#include <Data_reader2buffer.h>
#include <Buffer2data_writer.h>

// NGHK: remove
#include <Log_writer_cout.h>

int main(int argc, char *argv[]) {
  {
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
    
    remove(outfile);
  }
  
  { // Read from data_reader and write to data_writer using a buffer and two threads
    {
      typedef Buffer_element<char, 100>  value_type;
      boost::shared_ptr<Data_reader_file> 
        reader(new Data_reader_file(infile));
      boost::shared_ptr< Semaphore_buffer<value_type> > 
        buffer(new Semaphore_buffer<value_type>(1000));
      boost::shared_ptr<Data_writer_file> 
        writer(new Data_writer_file(outfile));
      
      Log_writer_cout log_writer;
      
      Data_reader2buffer<value_type>     reader2buffer;
      Buffer2data_writer<value_type>     buffer2writer;
      
      reader2buffer.set_data_reader(reader);
      reader2buffer.set_buffer(buffer);
      reader2buffer.start();
  
      buffer2writer.set_data_writer(writer);
      buffer2writer.set_buffer(buffer);
      buffer2writer.start();
      
      while (!reader2buffer.get_data_reader()->eof()) {
        usleep(100000);
      }
      while (!buffer2writer.get_buffer()->empty()) {
        usleep(100000);
      }
    }
  
    //check output:
    std::string command = "diff ";
    command += infile; command += " "; command += outfile;
    int result = system(command.c_str());
  
    if (result != 0) {
      std::cout << "ERROR: Difference in files" << std::endl;
      exit(1);
    }
    
    //remove(outfile);
  }
  return 0;
}
