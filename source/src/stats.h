#pragma once
#include "utils.h"
#include <Arduino.h>

namespace cointhing {

class Stats {
public:
    static String toJson(bool withData);

    static String utcTime();
    static time_t localTimestamp();
    static String localTime();
    static String utcStart();

    static void reset();

    static bool fetchTimeAPI();
    static bool fetchWorldTimeAPI();

    static void inc_gecko_price_fetch();
    static void inc_gecko_chart_fetch();
    static void inc_gecko_price_fetch_fail();
    static void inc_gecko_chart_fetch_fail();
    static void inc_time_fetch();
    static void inc_time_fetch_fail();
    static void inc_settings_change();
    static void inc_server_requests();
    static void inc_wifi_got_ip();
    static void inc_wifi_sta_disconnected();

    static void inc_brownout_counter();
    static void inc_crash_counter();

    static time_t get_last_price_fetch() { return last_price_fetch; }
    static time_t get_last_chart_fetch() { return last_chart_fetch; }
    static time_t get_last_time_fetch() { return last_time_fetch; }
    static time_t get_last_wifi_got_ip() { return last_wifi_got_ip; }
    static time_t get_last_wifi_disconnect() { return last_wifi_disconnect; }

private:
    // time
    static String timezone;
    static time_t utc_time_start;
    static uint32_t raw_offset;
    static uint32_t dst_offset;

    // counter
    static uint32_t gecko_price_fetch;
    static uint32_t gecko_chart_fetch;
    static uint32_t gecko_price_fetch_fail;
    static uint32_t gecko_chart_fetch_fail;

    static uint32_t time_fetch;
    static uint32_t time_fetch_fail;

    static uint32_t settings_change;

    static uint32_t server_requests;

    static uint32_t wifi_got_ip;
    static uint32_t wifi_sta_disconnected;

    static time_t last_price_fetch;
    static time_t last_chart_fetch;
    static time_t last_time_fetch;
    static time_t last_wifi_got_ip;
    static time_t last_wifi_disconnect;

    static uint32_t brownout_counter;
    static uint32_t crash_counter;

    static SemaphoreHandle_t stats_sync_mutex;
};

extern Stats stats;

} // namespace cointhing
