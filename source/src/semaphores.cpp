#include "semaphores.h"

namespace cointhing {

SemaphoreHandle_t fetchPriceSemaphore;
SemaphoreHandle_t fetchChartSemaphore;
SemaphoreHandle_t displayNextSemaphore;

SemaphoreHandle_t dataMutex;

void createSemaphores()
{
    fetchPriceSemaphore = xSemaphoreCreateBinary();
    fetchChartSemaphore = xSemaphoreCreateBinary();
    displayNextSemaphore = xSemaphoreCreateBinary();

    dataMutex = xSemaphoreCreateRecursiveMutex();
}

} // namespace cointhing
