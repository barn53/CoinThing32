#include "tasks.h"
#include "display.h"
#include "gecko.h"
#include "main.h"
#include "web_server.h"

namespace cointhing {

TaskHandle_t highWaterMarkTaskHandle;
TaskHandle_t heartbeatTaskHandle;

void createHighWaterMarkTask()
{
    xTaskCreate(
        [](void*) {
            while (true) {
                TRC_I_PRINTF("displayTask hwm:       %u\n", uxTaskGetStackHighWaterMark(displayTaskHandle));
                TRC_I_PRINTF("geckoTask hwm:         %u\n", uxTaskGetStackHighWaterMark(geckoTaskHandle));
                TRC_I_PRINTF("highWaterMarkTask hwm: %u\n", uxTaskGetStackHighWaterMark(nullptr));
                TRC_I_PRINTF("minimum free size:     %u\n", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_32BIT));
                vTaskDelay(5000);
            }
        }, /* Task function. */
        "highWaterMarkTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &highWaterMarkTaskHandle /* Task handle to keep track of created task */);
}

void createHeartbeatTask()
{
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
