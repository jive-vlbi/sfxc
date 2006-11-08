/*
General function definitions

Author     : RHJ Oerlemans
StartDate  : 04-08-2006
Last change: 04-08-2006

*/
void askContinue(void);

int getLongVal(char *key, char *val, char *skey, int& sval);

int getINT64Val(char *key, char *val, char *skey, INT64& sval);

int getFloatVal(char *key, char *val, char *skey, float& sval);

int str2int(char *val, int& sval);

int irbit2(UINT32 *iseed);

