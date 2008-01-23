/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Tests several implementations of buffers.
 */

#include <iostream>
#include <pthread.h>


#include "semaphore_buffer.h"
#include "ring_buffer.h"

#define NElem 3
typedef Buffer_element<int, NElem> T;

Buffer<T> *buffer;
int Nmessages = 14;

void *produce(void *) {
  assert(buffer != NULL);
  int msg=0;

  // last message terminates the production
  for (int i=0; i<Nmessages-1; i++) {
    T &t = buffer->produce();
    for (int i=0; i<NElem; i++) {
      t[i] = msg++;
    }
    //std::cout << "PROD: " << (msg/NElem)%Nproduce << std::endl;
    buffer->produced(NElem);
  }
  buffer->produce();
  buffer->produced(0);
  return NULL;
}

void *consume(void *) {
  assert(buffer != NULL);
  int msg=0;
  int size;
  while (true) {
    T &t = buffer->consume(size);
    if (size == 0) return NULL;
    for (int i=0; i<NElem; i++) {
      //std::cout << t[i] << " " << msg << std::endl;
      assert(msg == t[i]);
      msg++;
    }
    buffer->consumed();
  }
  return NULL;
};

typedef Semaphore_buffer<T> Sem_buffer;
typedef Ring_buffer<T> Rng_buffer;

int main(int argc, char*argv) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  pthread_t prod_thread, cons_thread;
  {
    Nmessages = 13;
    Sem_buffer buff1(5);
    buffer = &buff1;
    pthread_create(&cons_thread, NULL, consume, NULL);
    pthread_create(&prod_thread, NULL, produce, NULL);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
  }
  {
    Nmessages = 13;
    Rng_buffer buff2(13);
    buffer = &buff2;
    pthread_create(&prod_thread, NULL, produce, NULL);
    sleep(1);
    pthread_create(&cons_thread, NULL, consume, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
  }
  exit(0);
}
