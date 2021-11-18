#pragma once
#include <Arduino.h>

namespace cointhing {

extern TaskHandle_t highWaterMarkTaskHandle;
extern TaskHandle_t heartbeatTaskHandle;

void createHighWaterMarkTask();
void createHeartbeatTask();

} // namespace cointhing
