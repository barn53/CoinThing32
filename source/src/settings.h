#pragma once
#include "utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>

#define GECKO_SETTINGS_FILE "/gecko.json"
#define FINNHUB_SETTINGS_FILE "/finnhub.json"
#define BRIGHTNESS_FILE "/brightness.json"

namespace cointhing {

extern SemaphoreHandle_t settingsMutex;

enum class ChartStyle : uint8_t {
    SIMPLE = 0,
    HIGH_LOW,
    HIGH_LOW_FIRST_LAST
};

enum class Swap : uint8_t {
    INTERVAL_1 = 0, // 5s (chart), 10s (coin)
    INTERVAL_2, // 30s
    INTERVAL_3, // 1min
    INTERVAL_4 // 5min
};

enum ChartPeriod : uint8_t {
    PERIOD_NONE = 0,
    PERIOD_24_H = 1,
    PERIOD_48_H = 2,
    PERIOD_30_D = 4,
    PERIOD_60_D = 8,
    ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
};

enum Mode : uint8_t {
    ONE_COIN = 1,
    TWO_COINS,
    MULTIPLE_COINS
};

struct Coin {
    String id;
    String symbol;
    String name;
};

struct Currency {
    String currency;
    String symbol;
};

class GeckoSettings {
public:
    friend class Settings;
    GeckoSettings();

    void clear();

    const String& coin(uint32_t index) const;
    const String& name(uint32_t index) const;
    const String& symbol(uint32_t index) const;

    const String& currency1() const;
    const String& currency1Symbol() const;
    String currency1Lower() const;
    const String& currency2() const;
    const String& currency2Symbol() const;
    String currency2Lower() const;

    const std::array<Currency, 2>& getCurrencies() const { return m_currencies; }

    const std::vector<Coin>& coins() const { return m_coins; }
    uint32_t numberCoins() const;

    Mode mode() const { return m_mode; }
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chartPeriod() const { return m_chart_period; }
    Swap swapInterval() const { return m_swap_interval; }
    ChartStyle chartStyle() const { return m_chart_style; }

    bool heartbeat() const { return m_heartbeat; }

private:
    uint32_t validCoinIndex(uint32_t index) const;

    std::vector<Coin> m_coins;
    std::array<Currency, 2> m_currencies;

    Mode m_mode { Mode::ONE_COIN };
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    Swap m_swap_interval { Swap::INTERVAL_1 };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };
};

class FinnhubSettings {
public:
    friend class Settings;
    FinnhubSettings();

    const std::vector<String>& symbols() const { return m_symbols; }
    const String& apiToken() const { return m_api_token; }

private:
    std::vector<String> m_symbols;
    String m_api_token;
};

class Settings {
public:
    Settings();

    void set(const char* json, String& savedToFile);

    void read();
    void deleteFile() const;

    void readBrightness();
    void setBrightness(uint8_t b);

    uint8_t brightness() const { return m_brightness; }

    const GeckoSettings& gecko() const { return m_gecko; }
    const FinnhubSettings& finnhub() const { return m_finnhub; }

private:
    void setGecko(DynamicJsonDocument& doc, bool toFile);
    void setFinnhub(DynamicJsonDocument& doc, bool toFile);

    void writeGecko() const;
    void writeFinnhub() const;

    GeckoSettings m_gecko;
    FinnhubSettings m_finnhub;
    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };
};

extern Settings settings;

} // namespace cointhing
