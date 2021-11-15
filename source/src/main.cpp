#include "events.h"
#include "gecko.h"
#include "semaphores.h"
#include "tasks.h"
#include "timers.h"
#include "wifi_utils.h"
#include <Arduino.h>
#include <SPIFFS.h>

#define SETTINGS R"({"mode":3,"coins":[{"id":"bitcoin","symbol":"BTC","name":"Bitcoin"},)"              \
                 R"({"id":"ethereum","symbol":"ETH","name":"Ethereum"},)"                               \
                 R"({"id":"ripple","symbol":"XRP","name":"XRP"},)"                                      \
                 R"({"id":"solana","symbol":"SOL","name":"Solana"},)"                                   \
                 R"({"id":"cardano","symbol":"ADA","name":"Cardano"},)"                                 \
                 R"({"id":"polkadot","symbol":"DOT","name":"Polkadot"},)"                               \
                 R"({"id":"dogecoin","symbol":"DOGE","name":"Dogecoin"},)"                              \
                 R"({"id":"shiba-inu","symbol":"SHIB","name":"Shiba Inu"},)"                           \
                 R"({"id":"terra-luna","symbol":"LUNA","name":"Terra"},)"                              \
                 R"({"id":"litecoin","symbol":"LTC","name":"Litecoin"}],)"                              \
                 R"("currencies":[{"currency":"EUR","symbol":"€"},{"currency":"USD","symbol":"$"}],)" \
                 R"("swap_interval":0,"chart_period":8,"chart_style":1,"number_format":1,"heartbeat":true})"

#define SETTINGS_2 R"({"mode":3,"coins":[{"id":"bitcoin","symbol":"BTC","name":"Bitcoin"}],)"             \
                   R"("currencies":[{"currency":"EUR","symbol":"€"},{"currency":"USD","symbol":"$"}],)" \
                   R"("swap_interval":0,"chart_period":8,"chart_style":1,"number_format":1,"heartbeat":true})"

using namespace cointhing;

void setup()
{
    Serial.begin(115200);
    SPIFFS.begin();

    createSemaphores();

    setupWiFi();

    createTasks();
    createEventLoop();
    createTimers();

    settings.set(SETTINGS);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
