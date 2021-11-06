#pragma once
#include "utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>

namespace cointhing {

class Settings;

class SettingsCoins {
public:
    struct Coin {
        String id;
        String symbol;
        String name;
    };

    struct Currency {
        String currency;
        String symbol;
    };

    SettingsCoins();

    SettingsCoins& operator=(const Settings& settings);

    void clear();

    const String& coin(uint32_t index) const;
    const String& name(uint32_t index) const;
    const String& symbol(uint32_t index) const;

    const String& currency1() const;
    const String& currency1Symbol() const;
    const String& currency2() const;
    const String& currency2Symbol() const;

    const std::vector<Coin>& coins() const { return m_coins; }
    uint32_t numberCoins() const;

private:
    uint32_t validCoinIndex(uint32_t index) const;

protected:
    std::vector<Coin> m_coins;
    std::array<Currency, 2> m_currencies;

    mutable SemaphoreHandle_t m_mutex;
};

class SettingsDisplay {
public:
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

    SettingsDisplay();

    Mode mode() const { return m_mode; }
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chartPeriod() const { return m_chart_period; }
    Swap swapInterval() const { return m_swap_interval; }
    ChartStyle chartStyle() const { return m_chart_style; }
    bool heartbeat() const { return m_heartbeat; }
    uint8_t brightness() const { return m_brightness; }

protected:
    Mode m_mode { Mode::ONE_COIN };
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    Swap m_swap_interval { Swap::INTERVAL_1 };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };
    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };
};

class Settings : public SettingsCoins, SettingsDisplay {
public:
    Settings();

    void set(const char* json);
    void read();
    void write() const;
    void deleteFile() const;

    bool valid() const;

    void readBrightness();
    void setBrightness(uint8_t b);

private:
    void set(DynamicJsonDocument& doc, bool toFile);
    void trace() const;
};

extern Settings settings;

} // namespace cointhing
