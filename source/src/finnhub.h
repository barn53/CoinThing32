#pragma once
#include "settings.h"
#include <Arduino.h>
#include <atomic>
#include <map>

namespace cointhing {

enum class FinnhubRemit : uint32_t {
    settingsChanged = 1,
    fetchPrices,
    fetchCharts,
};

class Finnhub {
public:
    struct StockPrices {
        float current;
        float change;
        float percentChange;
        float highDay;
        float lowDay;
        float openDay;
        float closePrevious;
        time_t timestamp;
    };

    Finnhub();

    void fetchPrices();
    void fetchCharts();
    bool valid() const;

    const std::map<String, StockPrices>& getPrices() const;
    const std::map<String, std::vector<float>>& getChartData() const;

    void newSettings();
    const FinnhubSettings& getSettings() const;

    void cancel() const;
    void unCancel() const;

    String toJson() const;

private:
    // These settings, prices and chart data are in sync by finnhubSyncMutex
    FinnhubSettings m_settings;
    std::map<String, StockPrices> m_prices;
    std::map<String, std::vector<float>> m_chart_data;

    mutable std::atomic_bool m_cancel { false };
};

extern Finnhub finnhub;
extern SemaphoreHandle_t finnhubSyncMutex;
extern QueueHandle_t finnhubQueue;
extern TaskHandle_t finnhubTaskHandle;

void createFinnhubTask();

} // namespace cointhing
