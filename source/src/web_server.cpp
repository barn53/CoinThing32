#include "web_server.h"
#include "settings.h"
#include "stats.h"
#include "tasks.h"
#include "trace.h"
#include "utils.h"

#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <WiFi.h>

namespace cointhing {

AsyncWebServer server(80);

String getContentType(const String& filename)
{
    if (filename.endsWith(".html")) {
        return F("text/html");
    } else if (filename.endsWith(".css")) {
        return F("text/css");
    } else if (filename.endsWith(".json")) {
        return F("application/json");
    } else if (filename.endsWith(".js")) {
        return F("application/javascript");
    } else if (filename.endsWith(".ico")) {
        return F("image/x-icon");
    } else if (filename.endsWith(".jpg")) {
        return F("image/jpeg");
    } else if (filename.endsWith(".bmp")) {
        return F("image/bmp");
    }
    return F("text/plain");
}

bool streamFile(AsyncWebServerRequest* request, const char* filename)
{
    TRC_I_FUNC
    TRC_I_PRINT("Stream file: ")
    TRC_I_PRINT(filename)

    String contentType = getContentType(filename);
    String filename_found;
    String filename_gz(filename);
    filename_gz += ".gz";

    bool gzip(false);
    if (SPIFFS.exists(filename_gz)) {
        TRC_I_PRINT(" - found .gz: ")
        TRC_I_PRINT(filename_gz)
        filename_found = filename_gz;
        gzip = true;
    } else if (SPIFFS.exists(filename)) {
        filename_found = filename;
    } else {
        return false;
    }

    AsyncWebServerResponse* response(request->beginResponse(SPIFFS, filename_found, contentType));
    response->addHeader(F("Access-Control-Allow-Origin"), "*");
    if (gzip) {
        response->addHeader(F("Content-Encoding"), "gzip");
    }
    request->send(response);

    TRC_I_PRINTLN(" - ok");
    return true;
}

bool streamFile(AsyncWebServerRequest* request)
{
    TRC_I_FUNC
    String filename(request->url());
    if (filename.endsWith("/")) {
        filename += F("settings.html");
    }
    return streamFile(request, filename.c_str());
}

bool handleSet(AsyncWebServerRequest* request)
{
#if SERIAL_TRACE > 0
    TRC_I_PRINTLN("handleSet: parsed Query:");
    for (int ii = 0; ii < request->args(); ++ii) {
        TRC_I_PRINT(request->argName(ii));
        TRC_PRINT(" -> ");
        TRC_PRINTLN(request->arg(ii));
    }
#endif

    if (request->hasArg(F("brightness"))) {
        settings.setBrightness(static_cast<uint8_t>(request->arg(F("brightness")).toInt()));
        streamFile(request, BRIGHTNESS_FILE);
    } else if (request->hasArg(F("json"))) {
        settings.set(request->arg(F("json")).c_str());
        streamFile(request, SETTINGS_FILE);
    } else {
        request->send(200, F("application/json"), F(R"({"error":"Nothing to set!"})"));
    }

    return true;
}

bool handleStats(AsyncWebServerRequest* request)
{
    request->send(200, F("application/json"), stats.toJson());
    return true;
}

bool handleAction(AsyncWebServerRequest* request)
{
    TRC_I_FUNC
    String path(request->url());
    TRC_I_PRINTF("handleAction: path: %s\n", path.c_str());

    if (path == F("/action/set")) {
        return handleSet(request);
    } else if (path == F("/stats")) {
        return handleStats(request);
    } else if (path == F("/crash")) {
        auto i(7 / 0);
        return false;
        // } else if (path == F("/action/reset/esp")) {
        //     return handleResetESP();
        // } else if (path == F("/action/reset/settings")) {
        //     return handleResetSettings();
        // } else if (path == F("/action/reset/wifi")) {
        //     return handleResetWiFi();
        // } else if (path == F("/action/reset/all")) {
        //     return handleResetAll();
        // } else if (path == F("/action/reset/forupdate")) {
        //     return handleForUpdate();
        // } else if (path == F("/action/selftest")) {
        //     return handleSelftest();
        // } else if (path == F("/action/get/version")) {
        //     return handleGetVersion();
        // } else if (path == F("/action/get/name")) {
        //     return handleGetName();
        // } else if (path == F("/action/get/price")) {
        //     return handleGetPrice();
    }

    return false;
}

void createServer()
{
    server.onNotFound([](AsyncWebServerRequest* request) { // If the client requests any URI
        stats.inc_server_requests();
        if (!handleAction(request)
            && !streamFile(request)) {
            request->send(404, F("text/plain"), F("404: Not Found"));
        }
    });

    server.begin();
}

} // namespace cointhing
