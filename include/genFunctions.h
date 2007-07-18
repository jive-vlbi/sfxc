/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * General purpose function definitions. Non class specific
 */

#include <string>
#include <types.h>
#include "Log_writer.h"

int getLongVal(char *key, char *val, char *skey, int& sval, Log_writer &log_writer);

int getint64_tVal(char *key, char *val, char *skey, int64_t& sval, Log_writer &log_writer);

int getFloatVal(char *key, char *val, char *skey, float& sval, Log_writer &log_writer);

int getDoubleVal(char *key, char *val, char *skey, double& sval, Log_writer &log_writer);

int str2int(char *val, int& sval);

long str_to_long (std::string inString, int pos, int length);

void set_seed(unsigned long seed_);
int irbit2();
