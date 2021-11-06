#pragma once
#include <Arduino.h>
#include <WiFi.h>

namespace cointhing {

void wifiSleep();
void wifiWake();
void setupWiFi();

} // namespace cointhing
