#include "wifi_utils.h"
#include "events.h"
#include "secrets.h"
#include "stats.h"
#include "utils.h"

namespace cointhing {

void wifiSleep()
{
    TraceFunction;
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.setSleep(true);
    delay(1);
}

void wifiWake()
{
    TraceFunction;
    WiFi.setSleep(false);
    delay(1);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.enableSTA(true);
    TraceIPrint("WiFi mode: ");
    switch (WiFi.getMode()) {
    case WIFI_MODE_NULL:
        TracePrintln("WIFI_MODE_NULL");
        break;
    case WIFI_MODE_STA:
        TracePrintln("WIFI_MODE_STA");
        break;
    case WIFI_MODE_AP:
        TracePrintln("WIFI_MODE_AP");
        break;
    case WIFI_MODE_APSTA:
        TracePrintln("WIFI_MODE_APSTA");
        break;
    case WIFI_MODE_MAX:
        TracePrintln("WIFI_MODE_MAX");
        break;
    }
}

void wifiEventHandler(system_event_id_t event)
{
    TraceFunction;
    TraceIPrintf("System (WiFi) event: %u\n", event);

    switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        stats.inc_wifi_got_ip();
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdWiFiGotIP, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
        TraceIPrintf("Got IP address: %s\n", WiFi.localIP().toString().c_str());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        stats.inc_wifi_sta_disconnected();
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdWiFiDisconnected, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
        TraceIPrintln("WiFi disconnected");
        break;
    default:
        break;
    }
}

void setupWiFi()
{
    TraceFunction;
    WiFi.onEvent(wifiEventHandler);

    wifiWake();

    if (!WiFi.setHostname(HOST_NAME)) {
        TraceIPrintln("setHostname() failed!");
    }
    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);

    TraceIPrint("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        TracePrint(".");
        delay(100);
    }

    TraceIPrintln("Connected");
    TraceIPrintf(" IP address: %s\n", WiFi.localIP().toString().c_str());
    TraceIPrintf(" Hostname: %s\n", WiFi.getHostname());
    TraceIPrintf(" MAC: %s\n", WiFi.macAddress().c_str());
    TraceIPrintf(" Signal: %d dB\n", WiFi.RSSI());
}

} // namespace cointhing
