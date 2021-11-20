#include "wifi_utils.h"
#include "events.h"
#include "secrets.h"
#include "stats.h"
#include "utils.h"

namespace cointhing {

void wifiSleep()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.setSleep(true);
    delay(1);
}

void wifiWake()
{
    WiFi.setSleep(false);
    delay(1);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.enableSTA(true);
    TRC_I_PRINT("WiFi mode: ");
    switch (WiFi.getMode()) {
    case WIFI_MODE_NULL:
        TRC_PRINTLN("WIFI_MODE_NULL");
        break;
    case WIFI_MODE_STA:
        TRC_PRINTLN("WIFI_MODE_STA");
        break;
    case WIFI_MODE_AP:
        TRC_PRINTLN("WIFI_MODE_AP");
        break;
    case WIFI_MODE_APSTA:
        TRC_PRINTLN("WIFI_MODE_APSTA");
        break;
    case WIFI_MODE_MAX:
        TRC_PRINTLN("WIFI_MODE_MAX");
        break;
    }
}

void wifiEventHandler(system_event_id_t event)
{
    TRC_I_FUNC
    switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
        stats.inc_wifi_sta_connected();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        stats.inc_wifi_sta_disconnected();
        TRC_I_PRINT("Disconnected - try to reconnect ");
        while (WiFi.status() != WL_CONNECTED) {
            TRC_PRINT(".");
            WiFi.reconnect();
            delay(500);
        }
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdWiFiReconnected, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
        break;
    default:
        break;
    }
}

void setupWiFi()
{
    TRC_I_FUNC
    WiFi.onEvent(wifiEventHandler);

    wifiWake();

    if (!WiFi.setHostname(HOST_NAME)) {
        TRC_I_PRINTLN("setHostname() failed!");
    }
    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);

    TRC_I_PRINT("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        TRC_PRINT(".");
        delay(100);
    }

    TRC_PRINTLN("");
    TRC_I_PRINTLN("Connected");
    TRC_I_PRINTF(" IP address: %s\n", WiFi.localIP().toString().c_str());
    TRC_I_PRINTF(" Hostname: %s\n", WiFi.getHostname());
    TRC_I_PRINTF(" MAC: %s\n", WiFi.macAddress().c_str());
    TRC_I_PRINTF(" Signal: %d dB\n", WiFi.RSSI());
}

} // namespace cointhing
