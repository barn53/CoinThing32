#include "trace.h"

#if SERIAL_TRACE > 0
int callDepth { 0 };
uint32_t lastIndentMillis { 0 };
SemaphoreHandle_t traceMutex(xSemaphoreCreateRecursiveMutex());
#endif
