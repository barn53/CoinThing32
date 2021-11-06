#include "tasks.h"
#include "gecko.h"
#include "web_server.h"

namespace cointhing {

void createTasks()
{
    createGeckoTask();
    createServerTask();
}

} // namespace cointhing
