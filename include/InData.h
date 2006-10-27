#include "gen_defines.h"

int FindOffsets(StaP StaPrms[], INT64 sliceStartByte[][NcoresMax],
  INT64 sliceStopByte[][NcoresMax]);

int FindHeaderMk4(StaP StaPrms[], int sn, INT64 offset, int& jsynch, INT64& usTime);

int checkStoptime(StaP StaPrms[], int sn, INT64 StartByte);

