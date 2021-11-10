#pragma once
#include "esp_event.h"
#include <Arduino.h>

namespace cointhing {

extern esp_event_loop_args_t loopArgs;
extern esp_event_loop_handle_t loopHandle;

ESP_EVENT_DECLARE_BASE(ESP_EVENT_COINTHING_BASE)

extern int32_t eventIdDisplay;
extern int32_t eventIdFetch;

void createEventLoop();

} // namespace cointhing
