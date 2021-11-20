#include "tasks.h"
#include "display.h"
#include "gecko.h"
#include "main.h"
#include "stats.h"
#include "web_server.h"

namespace cointhing {

TaskHandle_t housekeepingTaskHandle;
TaskHandle_t heartbeatTaskHandle;

void createHousekeepingTask()
{
    TRC_I_FUNC
    xTaskCreate(
        [](void*) {
            HousekeepingNotificationType notificationType;
            while (true) {
                if (xTaskNotifyWait(0, 0xffffffff, reinterpret_cast<uint32_t*>(&notificationType), portMAX_DELAY)) {
                    TRC_I_PRINTF("Notification type: %u\n", static_cast<uint32_t>(notificationType));
                    switch (notificationType) {
                    case HousekeepingNotificationType::fetchTime:
                        if (!stats.fetchWorldTimeAPI()) {
                            xTaskNotify(housekeepingTaskHandle, static_cast<uint32_t>(HousekeepingNotificationType::fetchTime), eSetValueWithOverwrite);
                            delay(500);
                        }
                        break;
                    }
                }
            }
        }, /* Task function. */
        "housekeepingTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &housekeepingTaskHandle /* Task handle to keep track of created task */);

    xTaskNotify(housekeepingTaskHandle, static_cast<uint32_t>(HousekeepingNotificationType::fetchTime), eSetValueWithOverwrite);
}

void createHeartbeatTask()
{
    TRC_I_FUNC
    pinMode(2, OUTPUT); // on board led
    xTaskCreate(
        [](void*) {
            uint8_t counter(0);
            while (true) {
                counter %= 9;
                switch (counter) {
                case 0:
                case 3:
                    digitalWrite(2, 1);
                    break;
                default:
                    digitalWrite(2, 0);
                    break;
                }
                ++counter;
                vTaskDelay(100);
            }
        }, /* Task function. */
        "heartbeatTask", /* name of task. */
        2048, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &heartbeatTaskHandle /* Task handle to keep track of created task */);
}

} // namespace cointhing
