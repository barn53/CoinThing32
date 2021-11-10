#include "events.h"
#include "semaphores.h"
#include "trace.h"

namespace cointhing {

esp_event_loop_args_t loopArgs;
esp_event_loop_handle_t loopHandle;

ESP_EVENT_DEFINE_BASE(ESP_EVENT_COINTHING_BASE)

int32_t eventIdDisplay { 42 };
int32_t eventIdFetch { 53 };

void createEventLoop()
{
    loopArgs = {
        .queue_size = 5,
        .task_name = "cointhingEvent",
        .task_priority = 0,
        .task_stack_size = 2048,
        .task_core_id = 0
    };

    esp_event_loop_create(&loopArgs, &loopHandle);

    esp_event_handler_register_with(
        loopHandle, ESP_EVENT_COINTHING_BASE, ESP_EVENT_ANY_ID, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            TRC_I_FUNC
            TRC_I_PRINTF("Event: %u from: %s\n", event_id, (const char*)event_data);
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, ESP_EVENT_COINTHING_BASE, eventIdDisplay, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            xSemaphoreGive(displaySemaphore);
        },
        nullptr);

    esp_event_handler_register_with(
        loopHandle, ESP_EVENT_COINTHING_BASE, eventIdFetch, [](void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            xSemaphoreGive(fetchSemaphore);
        },
        nullptr);
}

} // namespace cointhing
