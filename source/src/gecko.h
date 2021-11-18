#pragma once
#include "settings.h"
#include <Arduino.h>
#include <map>

namespace cointhing {

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

    const std::vector<CoinPrices>& getCoinPrices() const { return m_prices; }
    const std::map<String, std::vector<float>>& getChartData() const { return m_chart_data; }
    const SettingsCoins& getSettings() const { return m_settings; }

private:
    // These settings and values are consistent when synchronized with dataMutex
    SettingsCoins m_settings;
    std::vector<CoinPrices> m_prices;
    std::map<String, std::vector<float>> m_chart_data;
};

extern Gecko gecko;

extern TaskHandle_t geckoPriceTaskHandle;
extern TaskHandle_t geckoChartTaskHandle;

void createGeckoTasks();

} // namespace cointhing
