#include "tasks.h"
#include "display.h"
#include "gecko.h"
#include "web_server.h"

namespace cointhing {

void createTasks()
{
    createDisplayTask();
    createGeckoTask();
    createServerTask();
}

} // namespace cointhing
