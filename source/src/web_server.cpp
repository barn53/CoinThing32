#include "web_server.h"
#include "utils.h"

namespace cointhing {

WebServer server(80);

TaskHandle_t serverTaskHandle;

void serverTask(void*)
{
    while (true) {
        server.handleClient();
    }
}

void createServerTask()
{
    server.on("/gaga", []() {
        TRC_I_FUNC
        server.send(200, "text/plain", "selber gaga");
    });
    server.onNotFound([]() { // If the client requests any URI
        TRC_I_FUNC
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    server.begin(); // Actually start the server

    xTaskCreate(
        serverTask, /* Task function. */
        "serverTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &serverTaskHandle /* Task handle to keep track of created task */);
}

} // namespace cointhing
