#pragma once
#include <TFT_eSPI.h>

namespace cointhing {

enum class DisplayNotificationType : uint32_t {
    showNextId = 1,
    settingsChanged,
};

class Display {
public:
    Display();

    void resetCoinId();
    void nextCoinId();

    void clear() const;

    void show() const;
    void showNewSettings() const;

private:
    uint32_t m_display_coin_index;
    mutable bool m_clear_on_show { false };
};

extern TFT_eSPI tft;
extern Display display;
extern TaskHandle_t displayTaskHandle;

void createDisplayTask();

} // namespace cointhing
