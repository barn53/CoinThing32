#pragma once
#include <Arduino.h>

namespace cointhing {

extern TaskHandle_t highWaterMarkTaskHandle;
extern TaskHandle_t blinkyTaskHandle;

void createHighWaterMarkTask();
void createBlinkyTask();

} // namespace cointhing
