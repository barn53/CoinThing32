#pragma once
#include <Arduino.h>

#define TASK_STACK_SIZE (8192)

namespace cointhing {

extern SemaphoreHandle_t coinsMutex;

} // namespace cointhing
