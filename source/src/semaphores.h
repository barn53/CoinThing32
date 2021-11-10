#pragma once
#include <Arduino.h>

namespace cointhing {

extern SemaphoreHandle_t fetchSemaphore;
extern SemaphoreHandle_t displaySemaphore;

void createSemaphores();

} // namespace cointhing
