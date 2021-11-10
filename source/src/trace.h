#pragma once
#include <Arduino.h>

#if SERIAL_TRACE > 0
extern int callDepth;
extern uint32_t lastIndentMillis;
extern SemaphoreHandle_t traceMutex;

#define TRC_INDENT                                      \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    Serial.print("[");                                  \
    Serial.print(millis() / 1000);                      \
    Serial.print("] ");                                 \
    Serial.print("[");                                  \
    Serial.print((millis() - lastIndentMillis) / 1000); \
    Serial.print("] ");                                 \
    Serial.printf("%17s", pcTaskGetTaskName(nullptr));  \
    Serial.print("@");                                  \
    Serial.print(xPortGetCoreID());                     \
    Serial.print(" ");                                  \
    lastIndentMillis = millis();                        \
    for (int __dd = 0; __dd < callDepth; ++__dd) {      \
        Serial.print("|   ");                           \
    }                                                   \
    xSemaphoreGiveRecursive(traceMutex);

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

#define TRC_I_FUNC                                      \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    TRC_INDENT                                          \
    Depth __d;                                          \
    Serial.print("+ ");                                 \
    Serial.print(__PRETTY_FUNCTION__);                  \
    Serial.print(" @ ");                                \
    FILE_LINE                                           \
    xSemaphoreGiveRecursive(traceMutex);

#define TRC_PRINT(x)                                    \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    Serial.print(x);                                    \
    xSemaphoreGiveRecursive(traceMutex);
#define TRC_PRINTLN(x)                                  \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    Serial.println(x);                                  \
    xSemaphoreGiveRecursive(traceMutex);
#define TRC_PRINTF(f, ...)                              \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    Serial.printf(f, __VA_ARGS__);                      \
    xSemaphoreGiveRecursive(traceMutex);

#define TRC_I_PRINT(x)                                  \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    TRC_INDENT                                          \
    TRC_PRINT(x)                                        \
    xSemaphoreGiveRecursive(traceMutex);
#define TRC_I_PRINTLN(x)                                \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    TRC_INDENT                                          \
    TRC_PRINTLN(x)                                      \
    xSemaphoreGiveRecursive(traceMutex);
#define TRC_I_PRINTF(f, ...)                            \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    TRC_INDENT                                          \
    TRC_PRINTF(f, __VA_ARGS__)                          \
    xSemaphoreGiveRecursive(traceMutex);

#define FILE_LINE                                       \
    xSemaphoreTakeRecursive(traceMutex, portMAX_DELAY); \
    Serial.print(__FILE__);                             \
    Serial.print(":");                                  \
    Serial.println(__LINE__);                           \
    xSemaphoreGiveRecursive(traceMutex);

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
