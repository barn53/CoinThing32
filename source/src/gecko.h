#pragma once
#include "settings.h"
#include <Arduino.h>
#include <atomic>
#include <map>

namespace cointhing {

enum class GeckoNotificationType : uint32_t {
    settingsChanged,
    fetchPrices,
    fetchCharts,
};

class Gecko {
public:
    struct CoinPrices {
        float priceCurrency1;
        float priceCurrency2;
        float change24hCurrency1;
        float change24hCurrency2;
        float volume24hCurrency1;
        float volume24hCurrency2;
        float marketCapCurrency1;
        float marketCapCurrency2;
    };

    Gecko();

    bool fetchPrices();
    bool fetchCharts();
    bool valid() const;

    const std::vector<CoinPrices>& getCoinPrices() const;
    const std::map<String, std::vector<float>>& getChartData() const;

    void newSettings();
    const SettingsCoins& getSettingsCoins() const;

    void cancel() const { m_cancel.store(true); }

private:
    // These settings, prices and chart data are in sync by geckoSyncMutex
    SettingsCoins m_settings_coins;
    std::vector<CoinPrices> m_prices;
    std::map<String, std::vector<float>> m_chart_data;

    mutable std::atomic_bool m_cancel { false };
};

extern Gecko gecko;
extern SemaphoreHandle_t geckoSyncMutex;
extern TaskHandle_t geckoTaskHandle;

void createGeckoTask();

} // namespace cointhing
