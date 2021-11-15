#pragma once
#include <Arduino.h>

namespace cointhing {

extern TimerHandle_t fetchPriceTimer;
extern TimerHandle_t fetchChartTimer;
extern TimerHandle_t displayNextTimer;

void createTimers();

} // namespace cointhing
