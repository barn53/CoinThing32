#pragma once
#include <Arduino.h>

#if SERIAL_TRACE > 0
extern int callDepth;
extern uint32_t lastIndentMillis;

#define TRC_INDENT                                      \
    Serial.print("[");                                  \
    Serial.print(millis() / 1000);                      \
    Serial.print("] ");                                 \
    Serial.print("[");                                  \
    Serial.print((millis() - lastIndentMillis) / 1000); \
    Serial.print("] ");                                 \
    Serial.print("@");                                  \
    Serial.print(xPortGetCoreID());                     \
    Serial.print(" ");                                  \
    lastIndentMillis = millis();                        \
    for (int __dd = 0; __dd < callDepth; ++__dd) {      \
        Serial.print("|   ");                           \
    }

struct Depth {
    Depth()
    {
        ++callDepth;
    }
    ~Depth()
    {
        --callDepth;
    }
};

#define TRC_I_FUNC                     \
    TRC_INDENT                         \
    Depth __d;                         \
    Serial.print("+ ");                \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(" @ ");               \
    FILE_LINE

#define TRC_PRINT(x) \
    Serial.print(x);
#define TRC_PRINTLN(x) \
    Serial.println(x);
#define TRC_PRINTF(f, ...) \
    Serial.printf(f, __VA_ARGS__);

#define TRC_I_PRINT(x) \
    TRC_INDENT         \
    TRC_PRINT(x)
#define TRC_I_PRINTLN(x) \
    TRC_INDENT           \
    TRC_PRINTLN(x)
#define TRC_I_PRINTF(f, ...) \
    TRC_INDENT               \
    TRC_PRINTF(f, __VA_ARGS__)

#define FILE_LINE           \
    Serial.print(__FILE__); \
    Serial.print(":");      \
    Serial.println(__LINE__);

#else
#define TRC_INDENT
#define FILE_LINE
#define TRC_I_FUNC
#define TRC_PRINT(x)
#define TRC_PRINTLN(x)
#define TRC_PRINTF(...)
#define TRC_I_PRINT(x)
#define TRC_I_PRINTLN(x)
#define TRC_I_PRINTF(...)
#endif
