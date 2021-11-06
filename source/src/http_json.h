#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

namespace cointhing {

class HttpJson {
public:
    HttpJson();

    bool read(const char* url, DynamicJsonDocument& jsonDoc);
    bool read(const char* url, DynamicJsonDocument& jsonDoc, DynamicJsonDocument& jsonFilter);

private:
    WiFiClientSecure m_client;
    HTTPClient m_http;
};

extern HttpJson httpJson;

} // namespace cointhing
