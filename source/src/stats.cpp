#include "stats.h"
#include "display.h"
#include "gecko.h"
#include "http_json.h"
#include "tasks.h"
#include "tracer.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>
using File = fs::File;

namespace cointhing {

String Stats::timezone;
time_t Stats::utc_time_start;
uint32_t Stats::raw_offset;
uint32_t Stats::dst_offset;

RTC_NOINIT_ATTR uint32_t Stats::gecko_price_fetch;
RTC_NOINIT_ATTR uint32_t Stats::gecko_chart_fetch;
RTC_NOINIT_ATTR uint32_t Stats::gecko_price_fetch_fail;
RTC_NOINIT_ATTR uint32_t Stats::gecko_chart_fetch_fail;

RTC_NOINIT_ATTR uint32_t Stats::time_fetch;
RTC_NOINIT_ATTR uint32_t Stats::time_fetch_fail;

RTC_NOINIT_ATTR uint32_t Stats::settings_change;

RTC_NOINIT_ATTR uint32_t Stats::server_requests;

RTC_NOINIT_ATTR uint32_t Stats::wifi_sta_connected;
RTC_NOINIT_ATTR uint32_t Stats::wifi_sta_disconnected;

RTC_NOINIT_ATTR time_t Stats::last_price_fetch;
RTC_NOINIT_ATTR time_t Stats::last_chart_fetch;
RTC_NOINIT_ATTR time_t Stats::last_time_fetch;
RTC_NOINIT_ATTR time_t Stats::last_wifi_connect;
RTC_NOINIT_ATTR time_t Stats::last_wifi_disconnect;

RTC_NOINIT_ATTR uint32_t Stats::brownout_counter;
RTC_NOINIT_ATTR uint32_t Stats::crash_counter;

SemaphoreHandle_t Stats::stats_sync_mutex = xSemaphoreCreateRecursiveMutex();

Stats stats;

void Stats::reset()
{
    RecursiveMutexGuard(stats_sync_mutex);
    timezone.clear();
    utc_time_start = 0;
    raw_offset = 0;
    dst_offset = 0;

    gecko_price_fetch = 0;
    gecko_chart_fetch = 0;
    gecko_price_fetch_fail = 0;
    gecko_chart_fetch_fail = 0;

    time_fetch = 0;
    time_fetch_fail = 0;

    settings_change = 0;

    server_requests = 0;

    wifi_sta_connected = 0;
    wifi_sta_disconnected = 0;

    last_price_fetch = 0;
    last_chart_fetch = 0;
    last_time_fetch = 0;
    last_wifi_connect = 0;
    last_wifi_disconnect = 0;

    brownout_counter = 0;
    crash_counter = 0;
}

void Stats::inc_gecko_price_fetch()
{
    RecursiveMutexGuard(stats_sync_mutex);
    last_price_fetch = localTimestamp();
    ++gecko_price_fetch;
}
void Stats::inc_gecko_chart_fetch()
{
    RecursiveMutexGuard(stats_sync_mutex);
    last_chart_fetch = localTimestamp();
    ++gecko_chart_fetch;
}
void Stats::inc_gecko_price_fetch_fail()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++gecko_price_fetch_fail;
}
void Stats::inc_gecko_chart_fetch_fail()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++gecko_chart_fetch_fail;
}
void Stats::inc_time_fetch()
{
    RecursiveMutexGuard(stats_sync_mutex);
    last_time_fetch = localTimestamp();
    ++time_fetch;
}
void Stats::inc_time_fetch_fail()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++time_fetch_fail;
}
void Stats::inc_settings_change()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++settings_change;
}
void Stats::inc_server_requests()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++server_requests;
}
void Stats::inc_wifi_sta_connected()
{
    RecursiveMutexGuard(stats_sync_mutex);
    last_wifi_connect = localTimestamp();
    ++wifi_sta_connected;
}
void Stats::inc_wifi_sta_disconnected()
{
    RecursiveMutexGuard(stats_sync_mutex);
    last_wifi_disconnect = localTimestamp();
    ++wifi_sta_disconnected;
}
void Stats::inc_brownout_counter()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++brownout_counter;
}
void Stats::inc_crash_counter()
{
    RecursiveMutexGuard(stats_sync_mutex);
    ++crash_counter;
}

String Stats::toJson(bool withData)
{
    TraceFunction;
    RecursiveMutexGuard(stats_sync_mutex);
    String json("{");

    json += R"("network":{)";
    json += R"("host":")" + String(WiFi.getHostname()) + R"(")";
    json += R"(,"ip":")" + WiFi.localIP().toString() + R"(")";
    json += R"(,"MAC":")" + WiFi.macAddress() + R"(")";
    json += R"(,"rssi":")" + String(WiFi.RSSI()) + R"( dB")";
    json += "}"; // network

    json += R"(,"time":{)";
    json += R"("timezone":")" + timezone + R"(")";
    json += R"(,"started utc":")" + utcStart() + R"(")";
    json += R"(,"current utc":")" + utcTime() + R"(")";
    if (timezone != "UTC") {
        json += R"(,"local time":")" + localTime() + R"(")";
    }
    json += R"(,"current utc timestamp":)" + String(static_cast<uint32_t>(utc_time_start + (esp_timer_get_time() / 1000 / 1000)));
    json += R"(,"raw offset":)" + String(raw_offset);
    json += R"(,"dst offset":)" + String(dst_offset);
    json += "}"; // time

    json += R"(,"stats":{)";

    json += R"("counters":{)";
    json += R"("gecko price fetch":)" + String(gecko_price_fetch);
    json += R"(,"gecko price fetch fail":)" + String(gecko_price_fetch_fail);
    json += R"(,"gecko chart fetch":)" + String(gecko_chart_fetch);
    json += R"(,"gecko chart fetch fail":)" + String(gecko_chart_fetch_fail);
    json += R"(,"time fetch":)" + String(time_fetch);
    json += R"(,"time fetch fail":)" + String(time_fetch_fail);
    json += R"(,"settings change":)" + String(settings_change);
    json += R"(,"server requests":)" + String(server_requests);
    json += R"(,"wifi sta connected":)" + String(wifi_sta_connected);
    json += R"(,"wifi sta disconnected":)" + String(wifi_sta_disconnected);
    json += R"(,"brownouts":)" + String(brownout_counter);
    json += R"(,"crashes":)" + String(crash_counter);
    json += "}"; // counters

    json += R"(,"timestamps":{)";
    json += R"("last price fetch":")" + timeFromTimestamp(last_price_fetch) + R"(")";
    json += R"(,"last chart fetch":")" + timeFromTimestamp(last_chart_fetch) + R"(")";
    json += R"(,"last time fetch":")" + timeFromTimestamp(last_time_fetch) + R"(")";
    json += R"(,"last wifi connect":")" + timeFromTimestamp(last_wifi_connect) + R"(")";
    json += R"(,"last wifi disconnect":")" + timeFromTimestamp(last_wifi_disconnect) + R"(")";
    json += "}"; // timestamps

    json += R"(,"memory":{)";

    json += R"("task high water marks":{)";
    json += R"("displayTask":)" + String(uxTaskGetStackHighWaterMark(displayTaskHandle));
    json += R"(,"geckoTask":)" + String(uxTaskGetStackHighWaterMark(geckoTaskHandle));
    json += R"(,"housekeepingTask":)" + String(uxTaskGetStackHighWaterMark(housekeepingTaskHandle));
    json += R"(,"heartbeatTask":)" + String(uxTaskGetStackHighWaterMark(heartbeatTaskHandle));
    json += "}"; // task high water marks

    json += R"(,"heap":{)";
    json += R"("minimum free size":)" + String(heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_32BIT));
    json += "}"; // heap

    json += "}"; // memory

    json += "}"; // stats

    if (withData) {
        json += R"(,"data":)";
        json += gecko.toJson();

        if (SPIFFS.exists(SETTINGS_FILE)) {
            File file;
            file = SPIFFS.open(SETTINGS_FILE, "r");
            if (file) {
                json += R"(,"settings":)";
                json += file.readString();
            }
        }

        if (SPIFFS.exists(BRIGHTNESS_FILE)) {
            File file;
            file = SPIFFS.open(BRIGHTNESS_FILE, "r");
            if (file) {
                json += R"(,"brightness":)";
                json += file.readString();
            }
        }
    }

    json += "}"; // top level
    return json;
}

String Stats::utcTime()
{
    return timeFromTimestamp(utc_time_start + (esp_timer_get_time() / 1000 / 1000));
}

time_t Stats::localTimestamp()
{
    return (utc_time_start + dst_offset + raw_offset + (esp_timer_get_time() / 1000 / 1000));
}

String Stats::localTime()
{
    return timeFromTimestamp(localTimestamp());
}

String Stats::utcStart()
{
    return timeFromTimestamp(utc_time_start);
}

bool Stats::fetchTimeAPI()
{
    TraceFunction;
    DynamicJsonDocument doc(512);
    String url(F("https://www.timeapi.io/api/Time/current/zone?timeZone=UTC"));
    TraceIPrintln(url);

    if (httpJson.read(url.c_str(), doc)) {
        RecursiveMutexGuard(stats_sync_mutex);
        tm time;
        time.tm_year = (doc[F("year")].as<int>() - 1900) | 0;
        time.tm_mon = (doc[F("month")].as<int>() - 1) | 0;
        time.tm_mday = doc[F("day")].as<int>() | 0;
        time.tm_hour = doc[F("hour")].as<int>() | 0;
        time.tm_min = doc[F("minute")].as<int>() | 0;
        time.tm_sec = doc[F("seconds")].as<int>() | 0;
        timezone = "UTC";
        utc_time_start = mktime(&time) - (esp_timer_get_time() / 1000 / 1000);
        if (time.tm_year != 0) {
            inc_time_fetch();
            return true;
        }
    } else {
        TraceIPrintln("HTTP read failed!");
    }

    inc_time_fetch_fail();
    return false;
}

bool Stats::fetchWorldTimeAPI()
{
    TraceFunction;
    DynamicJsonDocument doc(1024);
    String url(F("http://worldtimeapi.org/api/ip"));
    TraceIPrintln(url);

    if (httpJson.readHTTP(url.c_str(), doc)) {
        RecursiveMutexGuard(stats_sync_mutex);
        dst_offset = doc[F("dst_offset")] | 0;
        raw_offset = doc[F("raw_offset")] | 0;
        timezone = doc[F("timezone")] | "";
        utc_time_start = (doc[F("unixtime")].as<uint32_t>() - (esp_timer_get_time() / 1000 / 1000)) | 0;
        if (timezone != "") {
            inc_time_fetch();
            return true;
        }
    } else {
        TraceIPrintln("HTTP read failed!");
    }

    inc_time_fetch_fail();
    return false;
}

} // namespace cointhing
