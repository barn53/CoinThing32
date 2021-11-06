#pragma once
#include <Arduino.h>

namespace cointhing {

extern TimerHandle_t fetchTimer;

void createTimers();

} // namespace cointhing
