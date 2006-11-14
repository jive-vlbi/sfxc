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
void askContinue(void);

int getLongVal(char *key, char *val, char *skey, int& sval);

int getINT64Val(char *key, char *val, char *skey, INT64& sval);

int getFloatVal(char *key, char *val, char *skey, float& sval);

int str2int(char *val, int& sval);

int irbit2(UINT32 *iseed);

