#include "settings.h"
#include "events.h"
#include "main.h"
#include "stats.h"
#include "trace.h"
#include <StreamUtils.h>

#include <FS.h>
#include <SPIFFS.h>
using File = fs::File;

#define MIN_BRIGHTNESS 10

#define JSON_DOCUMENT_CONFIG_SIZE 1536
#define JSON_DOCUMENT_BRIGHTNESS_SIZE 32

namespace cointhing {

Settings::Settings()
{
}

void Settings::read()
{
    TRC_I_FUNC
    RecursiveMutexGuard g(coinsMutex);
    if (SPIFFS.exists(SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(SETTINGS_FILE, "r");
        if (file) {
            TRC_I_PRINTLN("Read settings: " SETTINGS_FILE);
            DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
            ReadBufferingStream bufferedFile { file, 64 };

#if SERIAL_TRACE > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                set(doc, false);
            } else {
                TRC_I_PRINT(F("deserializeJson() failed: "));
                TRC_I_PRINTLN(error.f_str());
            }
            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
    }
}

void Settings::set(const char* json)
{
    TRC_I_FUNC
    DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (!error) {
        set(doc, true);
    } else {
        TRC_I_PRINT(F("deserializeJson() failed: "));
        TRC_I_PRINTLN(error.f_str());
    }
}

void Settings::set(DynamicJsonDocument& doc, bool toFile)
{
    TRC_I_FUNC
    RecursiveMutexGuard g(coinsMutex);
    m_coins.m_mode = static_cast<Mode>(doc[F("mode")] | static_cast<uint8_t>(Mode::ONE_COIN));
    m_coins.m_coins.clear();
    for (JsonObject elem : doc[F("coins")].as<JsonArray>()) {
        Coin c;
        c.id = elem[F("id")] | "";
        c.symbol = elem[F("symbol")] | "";
        c.name = elem[F("name")] | "";
        m_coins.m_coins.emplace_back(c);
    }

    size_t ii(0);
    for (JsonObject elem : doc[F("currencies")].as<JsonArray>()) {
        Currency c;
        c.currency = elem[F("currency")] | "";
        c.symbol = elem[F("symbol")] | c.currency;
        m_coins.m_currencies[ii] = c;
        ++ii;
        if (ii >= m_coins.m_currencies.size()) {
            break;
        }
    }

    m_coins.m_number_format = static_cast<NumberFormat>(doc[F("number_format")] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    m_coins.m_chart_period = doc[F("chart_period")] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    m_coins.m_swap_interval = static_cast<Swap>(doc[F("swap_interval")] | static_cast<uint8_t>(Swap::INTERVAL_1));
    m_coins.m_chart_style = static_cast<ChartStyle>(doc[F("chart_style")] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    m_coins.m_heartbeat = doc[F("heartbeat")] | true;

    trace();
    if (toFile) {
        write();
    }
    stats.inc_settings_change();
    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdSettingsChanged, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, portMAX_DELAY);
}

void Settings::write() const
{
    TRC_I_FUNC
    RecursiveMutexGuard g(coinsMutex);
    File file = SPIFFS.open(SETTINGS_FILE, "w");
    if (file) {
        file.printf(R"({"mode":%u,)", static_cast<uint8_t>(m_coins.m_mode));
        file.print(R"("coins":[)");
        bool first(true);
        for (const auto& c : m_coins.m_coins) {
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
        for (const auto& c : m_coins.m_currencies) {
            if (first) {
                first = false;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"({"currency":"%s","symbol":"%s"})", c.currency.c_str(), c.symbol.c_str());
        }
        file.print(R"(],)");
        file.printf(R"("swap_interval":%u,)", static_cast<uint8_t>(m_coins.m_swap_interval));
        file.printf(R"("chart_period":%u,)", static_cast<uint8_t>(m_coins.m_chart_period));
        file.printf(R"("chart_style":%u,)", static_cast<uint8_t>(m_coins.m_chart_style));
        file.printf(R"("number_format":%u,)", static_cast<uint8_t>(m_coins.m_number_format));
        file.printf(R"("heartbeat":%s)", m_coins.m_heartbeat ? "true" : "false");
        file.print(R"(})");
        file.close();
    }
}

void Settings::deleteFile() const
{
    SPIFFS.remove(SETTINGS_FILE);
    SPIFFS.remove(BRIGHTNESS_FILE);
}

bool Settings::valid() const
{
    return SPIFFS.exists(SETTINGS_FILE);
}

void Settings::trace() const
{
#if SERIAL_TRACE___ > 0
    RecursiveMutexGuard g(coinsMutex);
    TRC_I_PRINTF("Mode: >%u<\n", m_mode)
    TRC_I_PRINTLN("Coins:")
    for (const auto& c : m_coins) {
        TRC_I_PRINTF("id: >%s<, name: >%s<, symbol: >%s<, \n", c.id.c_str(), c.name.c_str(), c.symbol.c_str())
    }
    TRC_I_PRINTF("Currency:       >%s< >%s<\n", m_currencies[0].currency.c_str(), m_currencies[0].symbol.c_str())
    TRC_I_PRINTF("Currency 2:     >%s< >%s<\n", m_currencies[1].currency.c_str(), m_currencies[1].symbol.c_str())
    TRC_I_PRINTF("Number format:  >%u<\n", m_number_format)
    TRC_I_PRINTF("Chart period:   >%u<\n", m_chart_period)
    TRC_I_PRINTF("Swap interval:  >%u<\n", m_swap_interval)
    TRC_I_PRINTF("Chart style:    >%u<\n", m_chart_style)
    TRC_I_PRINTF("Heart beat:     >%s<\n", (m_heartbeat ? "true" : "false"))
#endif
}

void Settings::readBrightness()
{
    TRC_I_FUNC
    RecursiveMutexGuard g(coinsMutex);
    if (SPIFFS.exists(BRIGHTNESS_FILE)) {
        File file;
        file = SPIFFS.open(BRIGHTNESS_FILE, "r");
        if (file) {
            TRC_I_PRINTLN("Read brightness: " BRIGHTNESS_FILE);
            StaticJsonDocument<JSON_DOCUMENT_BRIGHTNESS_SIZE> doc;
            ReadBufferingStream bufferedFile { file, 64 };

#if SERIAL_TRACE > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                m_coins.m_brightness = doc[F("b")] | std::numeric_limits<uint8_t>::max();
            } else {
                m_coins.m_brightness = std::numeric_limits<uint8_t>::max();
            }
            file.close();
        }
    }
}

void Settings::setBrightness(uint8_t b)
{
    TRC_I_FUNC
    RecursiveMutexGuard g(coinsMutex);
    if (b >= MIN_BRIGHTNESS
        && b <= std::numeric_limits<uint8_t>::max()) {
        m_coins.m_brightness = b;
        File file = SPIFFS.open(BRIGHTNESS_FILE, "w");
        if (file) {
            file.printf(R"({"b":%u})", m_coins.m_brightness);
            file.close();
        }
    }
}

SettingsCoins::SettingsCoins()
{
}

void SettingsCoins::clear()
{
    m_coins.clear();
}

const String& SettingsCoins::coin(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].id;
}

const String& SettingsCoins::name(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].name;
}

const String& SettingsCoins::symbol(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].symbol;
}

const String& SettingsCoins::currency1() const
{
    return m_currencies[0].currency;
}

const String& SettingsCoins::currency1Symbol() const
{
    return m_currencies[0].symbol;
}

String SettingsCoins::currency1Lower() const
{
    String l(m_currencies[0].currency);
    l.toLowerCase();
    return l;
}

const String& SettingsCoins::currency2() const
{
    return m_currencies[1].currency;
}

const String& SettingsCoins::currency2Symbol() const
{
    return m_currencies[1].symbol;
}

String SettingsCoins::currency2Lower() const
{
    String l(m_currencies[1].currency);
    l.toLowerCase();
    return l;
}

uint32_t SettingsCoins::numberCoins() const
{
    return m_coins.size();
}

uint32_t SettingsCoins::validCoinIndex(uint32_t index) const
{
    if (index >= m_coins.size()) {
        index = 0;
    }
    return index;
}

Settings settings;

} // namespace cointhing
