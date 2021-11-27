#include "web_server.h"
#include "settings.h"
#include "stats.h"
#include "tasks.h"
#include "tracer.h"
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
    TraceFunction;
    TraceIPrint("Stream file: ");
    TracePrintln(filename);

    String contentType = getContentType(filename);
    String filename_found;
    String filename_gz(filename);
    filename_gz += ".gz";

    bool gzip(false);
    if (SPIFFS.exists(filename_gz)) {
        TraceIPrint(" - found .gz: ");
        TracePrintln(filename_gz);
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

    TraceIPrintln(" - ok");
    return true;
}

bool streamFile(AsyncWebServerRequest* request)
{
    TraceFunction;
    String filename(request->url());
    if (filename.endsWith("/")) {
        filename += F("settings.html");
    }
    return streamFile(request, filename.c_str());
}

bool handleSet(AsyncWebServerRequest* request)
{
#if TRACER > 0
    TraceFunction;
    TraceIPrintln("handleSet: parsed Query:");
    for (int ii = 0; ii < request->args(); ++ii) {
        TraceIPrint(request->argName(ii));
        TracePrint(" -> ");
        TracePrintln(request->arg(ii));
    }
#endif

    if (request->hasArg(F("brightness"))) {
        settings.setBrightness(static_cast<uint8_t>(request->arg(F("brightness")).toInt()));
        streamFile(request, BRIGHTNESS_FILE);
    } else if (request->hasArg(F("json"))) {
        String savedToFile;
        settings.set(request->arg(F("json")).c_str(), savedToFile);
        streamFile(request, savedToFile.c_str());
    } else {
        request->send(200, F("application/json"), F(R"({"error":"Nothing to set!"})"));
    }

    return true;
}

bool handleStats(AsyncWebServerRequest* request, bool withData)
{
    request->send(200, F("application/json"), stats.toJson(withData));
    return true;
}

bool handleAction(AsyncWebServerRequest* request)
{
    TraceFunction;
    String path(request->url());
    TraceIPrintf("handleAction: path: %s\n", path.c_str());

    if (path == F("/action/set")) {
        return handleSet(request);
    } else if (path == F("/stats")) {
        return handleStats(request, false);
    } else if (path == F("/stats2")) {
        return handleStats(request, true);
    } else if (path == F("/wifioff")) {
        WiFi.setSleep(WIFI_PS_MAX_MODEM);
        return true;
    } else if (path == F("/crash")) {
        request->send(200, F("text/plain"), F("crashed!"));
        vTaskDelay(500);
        auto i(7 / 0);
        return true;
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
