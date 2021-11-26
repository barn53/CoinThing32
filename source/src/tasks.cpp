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
    TraceFunction;
    xTaskCreate(
        [](void*) {
            uint32_t minutes(0);
            while (true) {

                if (minutes % 60 == 0) {
                    stats.fetchWorldTimeAPI();
                }

                TraceNIPrintln(stats.toJson(false));

                ++minutes;
                vTaskDelay(60 * 1000);
            }
        }, /* Task function. */
        "housekeepingTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &housekeepingTaskHandle /* Task handle to keep track of created task */);
}

void createHeartbeatTask()
{
    TraceFunction;
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
