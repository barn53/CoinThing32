#pragma once
#include <Arduino.h>

namespace cointhing {

extern SemaphoreHandle_t fetchSemaphore;

void createSemaphores();

} // namespace cointhing
