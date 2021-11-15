#pragma once
#include <TFT_eSPI.h>

namespace cointhing {

class Display {
public:
    Display();

    void show(bool next);
    void clear();

private:
    void nextId();

    uint32_t displayId;
};

extern TFT_eSPI tft;
extern Display display;
extern TaskHandle_t displayTaskHandle;

void createDisplayTask();

} // namespace cointhing
