

#ifndef HA_test_h
#define HA_test_h

#include "Arduino.h"

extern int bill;

static const byte ERR_LIMIT = 16;
extern byte errorLog[];                           // Up to 16 hex codes stored
extern unsigned int numErrors;                         // Total count is available


void logError(byte errorCode);

#endif


