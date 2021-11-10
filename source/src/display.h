#pragma once
#include <TFT_eSPI.h>

namespace cointhing {

class Display {
public:
    Display();

    void show();
};

extern TFT_eSPI tft;
extern Display display;
extern TaskHandle_t displayTaskHandle;

void createDisplayTask();

} // namespace cointhing
