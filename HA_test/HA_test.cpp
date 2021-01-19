


#include <arduino.h>
#include "HA_test.h"

byte errorLog[ERR_LIMIT];                           // Up to 16 hex codes stored
unsigned int numErrors = 0;                         // Total count is available


void logError(byte errorCode) {
    errorLog[numErrors] = errorCode;
    if (numErrors++ >= ERR_LIMIT) numErrors--;
}
