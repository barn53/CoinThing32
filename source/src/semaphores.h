#pragma once
#include <Arduino.h>

namespace cointhing {

extern SemaphoreHandle_t fetchPriceSemaphore;
extern SemaphoreHandle_t fetchChartSemaphore;
extern SemaphoreHandle_t displayNextSemaphore;

extern SemaphoreHandle_t dataMutex;

void createSemaphores();

} // namespace cointhing
