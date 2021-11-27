#include "settings.h"
#include "events.h"
#include "main.h"
#include "stats.h"
#include "tracer.h"
#include <StreamUtils.h>

#include <FS.h>
#include <SPIFFS.h>
using File = fs::File;

#define MIN_BRIGHTNESS 10

#define JSON_DOCUMENT_CONFIG_SIZE 1536
#define JSON_DOCUMENT_BRIGHTNESS_SIZE 32

namespace cointhing {

SemaphoreHandle_t settingsMutex = xSemaphoreCreateRecursiveMutex();

Settings::Settings()
{
}

void Settings::read()
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);

    if (SPIFFS.exists(GECKO_SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(GECKO_SETTINGS_FILE, "r");
        if (file) {
            TraceIPrintln("Read settings: " GECKO_SETTINGS_FILE);
            DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
            ReadBufferingStream bufferedFile { file, 64 };

#if TRACER > 1
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                setGecko(doc, false);
            } else {
                TraceIPrint(F("deserializeJson() failed: "));
                TraceIPrintln(error.f_str());
            }
            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
    }

    if (SPIFFS.exists(FINNHUB_SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(FINNHUB_SETTINGS_FILE, "r");
        if (file) {
            TraceIPrintln("Read settings: " FINNHUB_SETTINGS_FILE);
            DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
            ReadBufferingStream bufferedFile { file, 64 };

#if TRACER > 1
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                setFinnhub(doc, false);
            } else {
                TraceIPrint(F("deserializeJson() failed: "));
                TraceIPrintln(error.f_str());
            }
            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
    }

    stats.inc_settings_change();
    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdSettingsChanged, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
}

void Settings::set(const char* json, String& savedToFile)
{
    TraceFunction;
    DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (!error) {
        if (doc[F("finnhub")].as<bool>() == true) {
            setFinnhub(doc, true);
            savedToFile = FINNHUB_SETTINGS_FILE;
        } else {
            setGecko(doc, true);
            savedToFile = GECKO_SETTINGS_FILE;
        }
        stats.inc_settings_change();
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdSettingsChanged, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
    } else {
        savedToFile.clear();
        TraceIPrint(F("deserializeJson() failed: "));
        TraceIPrintln(error.f_str());
    }
}

void Settings::setGecko(DynamicJsonDocument& doc, bool toFile)
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    m_gecko.m_mode = static_cast<Mode>(doc[F("mode")] | static_cast<uint8_t>(Mode::ONE_COIN));
    m_gecko.m_coins.clear();
    for (JsonObject elem : doc[F("coins")].as<JsonArray>()) {
        Coin c;
        c.id = elem[F("id")] | "";
        c.symbol = elem[F("symbol")] | "";
        c.name = elem[F("name")] | "";
        m_gecko.m_coins.emplace_back(c);
    }

    size_t ii(0);
    for (JsonObject elem : doc[F("currencies")].as<JsonArray>()) {
        Currency c;
        c.currency = elem[F("currency")] | "";
        c.symbol = elem[F("symbol")] | c.currency;
        m_gecko.m_currencies[ii] = c;
        ++ii;
        if (ii >= m_gecko.m_currencies.size()) {
            break;
        }
    }

    m_gecko.m_number_format = static_cast<NumberFormat>(doc[F("number_format")] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    m_gecko.m_chart_period = doc[F("chart_period")] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    m_gecko.m_swap_interval = static_cast<Swap>(doc[F("swap_interval")] | static_cast<uint8_t>(Swap::INTERVAL_1));
    m_gecko.m_chart_style = static_cast<ChartStyle>(doc[F("chart_style")] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    m_gecko.m_heartbeat = doc[F("heartbeat")] | true;

    if (toFile) {
        writeGecko();
    }
}

void Settings::writeGecko() const
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    File file = SPIFFS.open(GECKO_SETTINGS_FILE, "w");
    if (file) {
        file.printf(R"({"mode":%u,)", static_cast<uint8_t>(m_gecko.m_mode));
        file.print(R"("coins":[)");
        bool first(true);
        for (const auto& c : m_gecko.m_coins) {
            if (first) {
                first = false;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"({"id":"%s","symbol":"%s","name":"%s"})", c.id.c_str(), c.symbol.c_str(), c.name.c_str());
        }
        file.print(R"(],)");

        file.print(R"("currencies":[)");
        first = true;
        for (const auto& c : m_gecko.m_currencies) {
            if (first) {
                first = false;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"({"currency":"%s","symbol":"%s"})", c.currency.c_str(), c.symbol.c_str());
        }
        file.print(R"(],)");
        file.printf(R"("swap_interval":%u,)", static_cast<uint8_t>(m_gecko.m_swap_interval));
        file.printf(R"("chart_period":%u,)", static_cast<uint8_t>(m_gecko.m_chart_period));
        file.printf(R"("chart_style":%u,)", static_cast<uint8_t>(m_gecko.m_chart_style));
        file.printf(R"("number_format":%u,)", static_cast<uint8_t>(m_gecko.m_number_format));
        file.printf(R"("heartbeat":%s)", m_gecko.m_heartbeat ? "true" : "false");
        file.print(R"(})");
        file.close();
    }
}

void Settings::setFinnhub(DynamicJsonDocument& doc, bool toFile)
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    m_finnhub.m_symbols.clear();
    for (const char* symbol : doc[F("symbols")].as<JsonArray>()) {
        m_finnhub.m_symbols.emplace_back(symbol);
    }
    m_finnhub.m_api_token = doc[F("token")] | "";

    if (toFile) {
        writeFinnhub();
    }
}

void Settings::writeFinnhub() const
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    File file = SPIFFS.open(FINNHUB_SETTINGS_FILE, "w");
    if (file) {
        file.printf(R"({"finnhub":true)");
        file.print(R"(,"symbols":[)");
        bool once(false);
        for (const auto& s : m_finnhub.m_symbols) {
            if (!once) {
                once = true;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"("%s")", s.c_str());
        }
        file.print("]"); // symbols
        file.printf(R"(,"token":"%s")", m_finnhub.m_api_token.c_str());
        file.print("}"); // top level
        file.close();
    }
}

void Settings::deleteFile() const
{
    SPIFFS.remove(GECKO_SETTINGS_FILE);
    SPIFFS.remove(FINNHUB_SETTINGS_FILE);
    SPIFFS.remove(BRIGHTNESS_FILE);
}

void Settings::readBrightness()
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    if (SPIFFS.exists(BRIGHTNESS_FILE)) {
        File file;
        file = SPIFFS.open(BRIGHTNESS_FILE, "r");
        if (file) {
            TraceIPrintln("Read brightness: " BRIGHTNESS_FILE);
            StaticJsonDocument<JSON_DOCUMENT_BRIGHTNESS_SIZE> doc;
            ReadBufferingStream bufferedFile { file, 64 };

#if TRACER > 1
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                m_brightness = doc[F("b")] | std::numeric_limits<uint8_t>::max();
            } else {
                m_brightness = std::numeric_limits<uint8_t>::max();
            }
            file.close();
        }
    }
}

void Settings::setBrightness(uint8_t b)
{
    TraceFunction;
    RecursiveMutexGuard(settingsMutex);
    if (b >= MIN_BRIGHTNESS
        && b <= std::numeric_limits<uint8_t>::max()) {
        m_brightness = b;
        File file = SPIFFS.open(BRIGHTNESS_FILE, "w");
        if (file) {
            file.printf(R"({"b":%u})", m_brightness);
            file.close();
        }
    }
}

GeckoSettings::GeckoSettings()
{
}

void GeckoSettings::clear()
{
    m_coins.clear();
}

const String& GeckoSettings::coin(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].id;
}

const String& GeckoSettings::name(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].name;
}

const String& GeckoSettings::symbol(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].symbol;
}

const String& GeckoSettings::currency1() const
{
    return m_currencies[0].currency;
}

const String& GeckoSettings::currency1Symbol() const
{
    return m_currencies[0].symbol;
}

String GeckoSettings::currency1Lower() const
{
    String l(m_currencies[0].currency);
    l.toLowerCase();
    return l;
}

const String& GeckoSettings::currency2() const
{
    return m_currencies[1].currency;
}

const String& GeckoSettings::currency2Symbol() const
{
    return m_currencies[1].symbol;
}

String GeckoSettings::currency2Lower() const
{
    String l(m_currencies[1].currency);
    l.toLowerCase();
    return l;
}

uint32_t GeckoSettings::numberCoins() const
{
    return m_coins.size();
}

uint32_t GeckoSettings::validCoinIndex(uint32_t index) const
{
    if (index >= m_coins.size()) {
        index = 0;
    }
    return index;
}

FinnhubSettings::FinnhubSettings()
{
}

Settings settings;

} // namespace cointhing
