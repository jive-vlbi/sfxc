/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - SCARIe-sfxc
 * This file contains:
 *   - declaration of a data_reader that from an udp socket.
 *     WARNING: this is not a realiable transmission system.
 *              the missing packets are replaced with a user
 *              provided function.
 */
#ifndef DATA_READER_UDP_HH
#define DATA_READER_UDP_HH

#include <cassert>
#include "simple_memory_pool.h"
#include "threadsafe_queue.h"
#include "network.h"
#include "data_reader.h"
#include "udp_packet.h"
#include "thread.h"

template<class T>
class Ring_buffer {
  unsigned int size_;

  std::vector<T> buffer_;
public:
  Ring_buffer(unsigned int size) {
    size_ = size;
    buffer_.resize(size);
  }

  void set_at(unsigned int idx, T& val) {
    buffer_[idx%size_] = val;
  }

  T& operator[](unsigned int i) {
    return buffer_[i%size_];
  }
};


class Data_reader_udp : public Data_reader {
public:
  Data_reader_udp(int socket);
  //Data_reader_udp(Connexion* connexion);

  virtual ~Data_reader_udp();
  virtual bool eof();
  bool can_read();

  void closef() {
    assert(false && "not implemented");
  }

protected:
  int do_get_bytes(size_t nBytes, char *buff);

  int socket_;
  int baseidx;
  bool isfirst;

  // the packet size. This is either
  // 1450 or with jumbo frame: 8950
  int psize_;
  int size_;

  typedef Simple_memory_pool<Packet_fixed_size>::pElement pool_element;
  Simple_memory_pool< Packet_fixed_size >  pool_;
  Threadsafe_queue<pool_element>    queue_;

class Reading_thread : public Thread {
    friend class Data_reader_udp;
    uint64_t packet_lost_;
    uint64_t packet_received_;
    uint64_t packet_dropped_;
    uint64_t packet_duplicated_;
    uint64_t packet_outoforder_;
    uint64_t packet_base_;
    // the packet size. This is either
    // 1450 or with jumbo frame: 8950
    int psize_;
    int size_;
    int socket_;
    int prev_packet_;


    Simple_memory_pool<Packet_fixed_size>& parent_pool_;
    Threadsafe_queue<pool_element>&        parent_queue_;

    Ring_buffer<pool_element>       ringbuffer_;

    void add_packet(int i, pool_element p);
  public:
    Reading_thread(Simple_memory_pool<Packet_fixed_size>& pool,
                   Threadsafe_queue<pool_element>& queue,
                   size_t psize, size_t size, int socket):
        parent_pool_(pool), parent_queue_(queue),
        ringbuffer_(size) {
      socket_ = socket;
      psize_ = psize;
      size_ = size;
      packet_lost_ = 0;
      packet_received_ = 0;
      packet_dropped_ = 0;
      packet_duplicated_ = 0;
      packet_base_ = 0;
      packet_outoforder_=0;
    }
    void do_execute();
  };

  Reading_thread reading_thread_;

  // used to handle half-filled packet.
  uint64_t old_current_idx_;
private:
};

#endif // DATAREADER_SOCKET_HH
