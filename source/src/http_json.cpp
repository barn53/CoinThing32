#include "http_json.h"
#include "utils.h"
#include <StreamUtils.h>

namespace cointhing {

HttpJson::HttpJson()
{
    m_http_mutex = xSemaphoreCreateRecursiveMutex();
    m_secure_client.setInsecure();
    m_http.useHTTP10(true); // stream is only available with HTTP1.0 (no chunked transfer)
    m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
}

bool HttpJson::read(const char* url, DynamicJsonDocument& jsonDoc)
{
    DynamicJsonDocument jsonFilter(0);
    jsonFilter.set(true);

    return read(url, jsonDoc, jsonFilter);
}

bool HttpJson::read(const char* url, DynamicJsonDocument& jsonDoc, DynamicJsonDocument& jsonFilter)
{
    TraceFunction;
    RecursiveMutexGuard(m_http_mutex);
    m_http.begin(m_secure_client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            ReadBufferingClient bufferedClient { m_secure_client, 64 };
            deserializeJson(jsonDoc, bufferedClient, DeserializationOption::Filter(jsonFilter));
            m_http.end();
            return true;
        } else {
            TraceIPrintf("[HTTP] GET... : %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
        }
    } else {
        TraceIPrintf("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
    }
    m_http.end();
    return false;
}

bool HttpJson::readHTTP(const char* url, DynamicJsonDocument& jsonDoc)
{
    TraceFunction;
    RecursiveMutexGuard(m_http_mutex);
    WiFiClient client;
    m_http.begin(client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            ReadBufferingClient bufferedClient { client, 64 };
            deserializeJson(jsonDoc, bufferedClient);
            m_http.end();
            return true;
        } else {
            TraceIPrintf("[HTTP] GET... : %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
        }
    } else {
        TraceIPrintf("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
    }
    m_http.end();
    return false;
}

HttpJson httpJson;

} // namespace cointhing
