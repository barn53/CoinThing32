#pragma once
#include <Arduino.h>
#include <atomic>

#define TASK_STACK_SIZE (8192)

namespace cointhing {

extern SemaphoreHandle_t settingsMutex;

extern std::atomic_bool readyFlag;

} // namespace cointhing
