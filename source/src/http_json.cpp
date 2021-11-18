#include "http_json.h"
#include "utils.h"
#include <StreamUtils.h>

namespace cointhing {

HttpJson::HttpJson()
{
    m_mutex = xSemaphoreCreateRecursiveMutex();
    m_client.setInsecure();
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
    TRC_I_FUNC
    RecursiveMutexGuard g(m_mutex);
    m_http.begin(m_client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            ReadBufferingClient bufferedClient { m_client, 64 };
            deserializeJson(jsonDoc, bufferedClient, DeserializationOption::Filter(jsonFilter));
            m_http.end();
            return true;
        } else {
            TRC_I_PRINTF("[HTTP] GET... : %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
        }
    } else {
        TRC_I_PRINTF("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
    }
    m_http.end();
    return false;
}

HttpJson httpJson;

} // namespace cointhing
