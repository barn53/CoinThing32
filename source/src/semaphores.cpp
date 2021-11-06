#include "semaphores.h"

namespace cointhing {

SemaphoreHandle_t fetchSemaphore;

void createSemaphores()
{
    fetchSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(fetchSemaphore);
}

} // namespace cointhing
