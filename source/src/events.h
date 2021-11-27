#pragma once
#include "esp_event.h"
#include <Arduino.h>

namespace cointhing {

extern esp_event_loop_handle_t loopHandle;

ESP_EVENT_DECLARE_BASE(COINTHING_EVENT_BASE)

extern int32_t eventIdStartCoinThing;
extern int32_t eventIdAllGeckoPricesUpdated;
extern int32_t eventIdGeckoChartUpdated;
extern int32_t eventIdAllGeckoChartsUpdated;
extern int32_t eventIdAllFinnhubPricesUpdated;
extern int32_t eventIdFinnhubChartUpdated;
extern int32_t eventIdAllFinnhubChartsUpdated;
extern int32_t eventIdSettingsChanged;
extern int32_t eventIdWiFiDisconnected;
extern int32_t eventIdWiFiGotIP;

void createEvents();
void registerEventHandler();

} // namespace cointhing
