#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

namespace cointhing {

extern AsyncWebServer server;

void createServer();

} // namespace cointhing

