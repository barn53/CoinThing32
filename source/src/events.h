#pragma once
#include "esp_event.h"
#include <Arduino.h>

namespace cointhing {

extern esp_event_loop_handle_t loopHandle;

ESP_EVENT_DECLARE_BASE(COINTHING_EVENT_BASE)

extern int32_t eventIdAllPricesUpdated;
extern int32_t eventIdChartUpdated;
extern int32_t eventIdAllChartsUpdated;
extern int32_t eventIdSettingsChanged;
extern int32_t eventIdWiFiReconnected;

void createEventLoop();
void registerEventHandler();

} // namespace cointhing
