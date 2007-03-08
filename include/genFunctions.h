/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

General function definitions

Author     : RHJ Oerlemans
StartDate  : 20060804
Last change: 20060804

*/

#include <string>
#include <types.h>
#include "Log_writer.h"

int getLongVal(char *key, char *val, char *skey, int& sval, Log_writer &log_writer);

int getINT64Val(char *key, char *val, char *skey, INT64& sval, Log_writer &log_writer);

int getFloatVal(char *key, char *val, char *skey, float& sval, Log_writer &log_writer);

int str2int(char *val, int& sval);

long str_to_long (std::string inString, int pos, int length);

int irbit2(UINT32 *iseed);

