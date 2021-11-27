#include "events.h"
#include "display.h"
#include "finnhub.h"
#include "gecko.h"
#include "main.h"
#include "tracer.h"

namespace cointhing {

esp_event_loop_handle_t loopHandle;

ESP_EVENT_DEFINE_BASE(COINTHING_EVENT_BASE)

int32_t eventIdStartCoinThing { 33 };

int32_t eventIdAllGeckoPricesUpdated { 42 };
int32_t eventIdGeckoChartUpdated { 53 };
int32_t eventIdAllGeckoChartsUpdated { 55 };

int32_t eventIdAllFinnhubPricesUpdated { 142 };
int32_t eventIdFinnhubChartUpdated { 153 };
int32_t eventIdAllFinnhubChartsUpdated { 155 };

int32_t eventIdSettingsChanged { 68 };
int32_t eventIdWiFiDisconnected { 191 };
int32_t eventIdWiFiGotIP { 192 };

void createEvents()
{
    TraceFunction;
    esp_event_loop_args_t loopArgs = {
        .queue_size = 5,
        .task_name = "cointhingEvent",
        .task_priority = 0,
        .task_stack_size = 4096,
        .task_core_id = 0
    };

    esp_event_loop_create(&loopArgs, &loopHandle);

    registerEventHandler();
}

void registerEventHandler()
{
    TraceFunction;
    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, ESP_EVENT_ANY_ID, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            TraceFunction;
            TraceIPrintf("Event: %s, id: %u data: %s\n", (const char*)event_base, event_id, (const char*)event_data);
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, eventIdStartCoinThing, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            TraceFunction;
            TraceIPrintln("Set ready flag");
            readyFlag = true;
            settings.read();
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, eventIdSettingsChanged, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            if (readyFlag) {
                {
                    xQueueReset(geckoQueue);
                    gecko.cancel();
                    GeckoRemit type(GeckoRemit::settingsChanged);
                    xQueueSend(geckoQueue, static_cast<void*>(&type), 0);
                }
                {
                    xQueueReset(finnhubQueue);
                    finnhub.cancel();
                    FinnhubRemit type(FinnhubRemit::settingsChanged);
                    xQueueSend(finnhubQueue, static_cast<void*>(&type), 0);
                }
                {
                    xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::settingsChanged), eSetValueWithOverwrite);
                }
            }
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, eventIdWiFiGotIP, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            if (readyFlag) {
                gecko.cancel();
                finnhub.cancel();
                settings.read();
            }
        },
        nullptr);
}

} // namespace cointhing
