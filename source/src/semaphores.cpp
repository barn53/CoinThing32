#include "semaphores.h"

namespace cointhing {

SemaphoreHandle_t fetchSemaphore;
SemaphoreHandle_t displaySemaphore;

void createSemaphores()
{
    fetchSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(fetchSemaphore);

    displaySemaphore = xSemaphoreCreateBinary();
}

} // namespace cointhing
