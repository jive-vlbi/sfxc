/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * General function definitions
 */

#include <string>
#include <types.h>
#include "Log_writer.h"

int getLongVal(char *key, char *val, char *skey, int& sval, Log_writer &log_writer);

int getINT64Val(char *key, char *val, char *skey, INT64& sval, Log_writer &log_writer);

int getFloatVal(char *key, char *val, char *skey, float& sval, Log_writer &log_writer);

int getDoubleVal(char *key, char *val, char *skey, double& sval, Log_writer &log_writer);

int str2int(char *val, int& sval);

long str_to_long (std::string inString, int pos, int length);

int irbit2(UINT32 *iseed);

