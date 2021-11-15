#pragma once
#include <Arduino.h>
#include <WebServer.h>

namespace cointhing {

extern WebServer server;
extern TaskHandle_t serverTaskHandle;

void createServerTask();

} // namespace cointhing
