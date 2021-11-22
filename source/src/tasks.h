#pragma once
#include <Arduino.h>

namespace cointhing {

extern TaskHandle_t housekeepingTaskHandle;
extern TaskHandle_t heartbeatTaskHandle;

void createHousekeepingTask();
void createHeartbeatTask();

} // namespace cointhing
