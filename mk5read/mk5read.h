#ifndef MK5READ_H
#define MK5READ_H

#include <stdint.h>

#define MK5READ_SOCKET "/tmp/mk5read"

struct mk5read_msg {
	char		vsn[9];
	uint64_t	off;
};

#endif
