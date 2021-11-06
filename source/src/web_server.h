#pragma once
#include <Arduino.h>
#include <WebServer.h>

namespace cointhing {

extern WebServer server;

void createServerTask();

} // namespace cointhing
