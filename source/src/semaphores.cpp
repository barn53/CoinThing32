#include "semaphores.h"

namespace cointhing {

SemaphoreHandle_t fetchPriceSemaphore;
SemaphoreHandle_t fetchChartSemaphore;
SemaphoreHandle_t displayNextSemaphore;

SemaphoreHandle_t coinsMutex;

void createSemaphores()
{
    fetchPriceSemaphore = xSemaphoreCreateBinary();
    fetchChartSemaphore = xSemaphoreCreateBinary();
    displayNextSemaphore = xSemaphoreCreateBinary();

    coinsMutex = xSemaphoreCreateRecursiveMutex();
}

} // namespace cointhing
