#include "events.h"
#include "display.h"
#include "gecko.h"
#include "trace.h"

namespace cointhing {

esp_event_loop_handle_t loopHandle;

ESP_EVENT_DEFINE_BASE(COINTHING_EVENT_BASE)

int32_t eventIdAllPricesUpdated { 42 };
int32_t eventIdChartUpdated { 53 };
int32_t eventIdAllChartsUpdated { 55 };
int32_t eventIdSettingsChanged { 68 };
int32_t eventIdWiFiReconnected { 192 };

void createEventLoop()
{
    TRC_I_FUNC
    esp_event_loop_args_t loopArgs = {
        .queue_size = 5,
        .task_name = "cointhingEvent",
        .task_priority = 0,
        .task_stack_size = 2048,
        .task_core_id = 0
    };

    esp_event_loop_create(&loopArgs, &loopHandle);
}

void registerEventHandler()
{
    TRC_I_FUNC
    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, ESP_EVENT_ANY_ID, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            TRC_I_FUNC
            TRC_I_PRINTF("Event: %s, id: %u data: %s\n", (const char*)event_base, event_id, (const char*)event_data);
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, eventIdSettingsChanged, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            gecko.cancel();
            xTaskNotify(geckoTaskHandle, static_cast<uint32_t>(GeckoNotificationType::settingsChanged), eSetValueWithOverwrite);
            xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::settingsChanged), eSetValueWithOverwrite);
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, COINTHING_EVENT_BASE, eventIdWiFiReconnected, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            gecko.cancel();
            settings.read();
            xTaskNotify(geckoTaskHandle, static_cast<uint32_t>(GeckoNotificationType::settingsChanged), eSetValueWithOverwrite);
        },
        nullptr);
}

} // namespace cointhing
