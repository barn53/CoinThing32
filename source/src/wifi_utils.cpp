#include "wifi_utils.h"
#include "secrets.h"
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

void setupWiFi()
{
    TRC_I_FUNC
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
