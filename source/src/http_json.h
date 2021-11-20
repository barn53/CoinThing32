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
    bool readHTTP(const char* url, DynamicJsonDocument& jsonDoc);

private:
    WiFiClientSecure m_secure_client;
    HTTPClient m_http;
    mutable SemaphoreHandle_t m_mutex;
};

extern HttpJson httpJson;

} // namespace cointhing
