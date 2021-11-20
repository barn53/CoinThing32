#pragma once
#include <Arduino.h>

namespace cointhing {

enum class HousekeepingNotificationType : uint32_t {
    fetchTime,
};

extern TaskHandle_t housekeepingTaskHandle;
extern TaskHandle_t heartbeatTaskHandle;

void createHousekeepingTask();
void createHeartbeatTask();

} // namespace cointhing
