/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 *
 * $Id$
 *
 * Definitions of common types.
 */

#ifndef TYPES_H
#define TYPES_H

#include "../config.h"
#include <stdint.h>

// For using large files:
#ifdef _FILE_OFFSET_BITS
#  if _FILE_OFFSET_BITS == 64
#    define FOPEN fopen
#  else
#    define FOPEN fopen64
#  endif
#else
#  define FOPEN fopen64
#endif

/**
    \defgroup ImportantClasses Important classes
    \defgroup Node             Nodes for the MPI-version
 **/
#endif // TYPES_H
