#pragma once
#include "settings.h"
#include <Arduino.h>
#include <atomic>
#include <map>

namespace cointhing {

enum class GeckoRemit : uint32_t {
    settingsChanged = 1,
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

    void fetchPrices();
    void fetchCharts();
    bool valid() const;

    const std::vector<CoinPrices>& getCoinPrices() const;
    const std::map<String, std::vector<float>>& getChartData() const;

    void newSettings();
    const GeckoSettings& getSettings() const;

    void cancel() const;
    void unCancel() const;

    String toJson() const;

private:
    // These settings, prices and chart data are in sync by geckoSyncMutex
    GeckoSettings m_settings;
    std::vector<CoinPrices> m_prices;
    std::map<String, std::vector<float>> m_chart_data;

    mutable std::atomic_bool m_cancel { false };
};

extern Gecko gecko;
extern SemaphoreHandle_t geckoSyncMutex;
extern QueueHandle_t geckoQueue;
extern TaskHandle_t geckoTaskHandle;

void createGeckoTask();

} // namespace cointhing
