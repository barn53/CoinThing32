#pragma once
#include <TFT_eSPI.h>

namespace cointhing {

enum class DisplayNotificationType : uint32_t {
    settingsChanged = 1,
    showNextId,
};

enum class DisplayCategory : uint32_t {
    None,
    Gecko,
    Finnhub,
};

enum class DisplayShown : uint32_t {
    NewSettings,
    Nothing,
    Gecko,
    Finnhub,
};

class Display {
public:
    Display();

    void resetIds();
    void nextId();

    void begin() const;

    void show() const;
    void showNewSettings() const;

private:
    void showGecko() const;
    void showFinnhub() const;
    void showNothing() const;

    uint32_t m_display_gecko_index;
    uint32_t m_display_finnhub_index;
    DisplayCategory m_display_category;
    mutable DisplayShown m_last_shown;
    static SemaphoreHandle_t m_tft_sync_mutex;
};

extern TFT_eSPI tft;
extern Display display;
extern TaskHandle_t displayTaskHandle;

void createDisplayTask();

} // namespace cointhing
