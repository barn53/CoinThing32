#pragma once
#include "settings.h"
#include <Arduino.h>
#include <map>

namespace cointhing {

struct CoinData {
    float m_price { 0. }; // price 1st currency
    float m_price_2nd { 0. }; // price 2nd currency
    float m_market_cap { 0. }; // market capitalization 1st currency
    std::vector<float> m_chart; // chart values
};

class Gecko {
public:
    Gecko();

    void set(const Settings& settings);
    void fetchAll();
    void clear();

private:
    void fetchOne(const SettingsCoins::Coin& coin);

    SettingsCoins m_settings; // copy of the coins-part of settings
    std::map<String, CoinData> m_coin_data; // first: id, second: data

    mutable SemaphoreHandle_t m_mutex;
};

extern Gecko gecko;

extern TaskHandle_t geckoTaskHandle;

void createGeckoTask();

} // namespace cointhing
