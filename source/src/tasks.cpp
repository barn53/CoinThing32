#include "tasks.h"
#include "display.h"
#include "gecko.h"
#include "web_server.h"

namespace cointhing {

TaskHandle_t highWaterMarkTaskHandle;

void createHighWaterMarkTask()
{
    xTaskCreate(
        [](void*) {
            while (true) {
                TRC_I_PRINTF("displayTask hwm:       %u\n", uxTaskGetStackHighWaterMark(displayTaskHandle));
                TRC_I_PRINTF("geckoPriceTask hwm:    %u\n", uxTaskGetStackHighWaterMark(geckoPriceTaskHandle));
                TRC_I_PRINTF("geckoChartTask hwm:    %u\n", uxTaskGetStackHighWaterMark(geckoChartTaskHandle));
                TRC_I_PRINTF("serverTask hwm:        %u\n", uxTaskGetStackHighWaterMark(serverTaskHandle));
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

void createTasks()
{
    createGeckoTasks();
    createDisplayTask();
    createServerTask();

    createHighWaterMarkTask();
}

} // namespace cointhing
