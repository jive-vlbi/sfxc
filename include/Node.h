#ifndef NODE_H
#define NODE_H

/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

//undef have to be before include <mpi.h>
#ifdef SEEK_SET
#undef SEEK_SET
#endif
#ifdef SEEK_END
#undef SEEK_END
#endif
#ifdef SEEK_CUR
#undef SEEK_CUR
#endif
#include <mpi.h>

#include <string>


class Node {
public:
  Node(int rank);
  
  void write_debug(const std::string &msg);
private:
  int rank;
};

#endif // NODE_H
