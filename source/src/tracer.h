#pragma once
#include <Arduino.h>
#include <map>

#if TRACER > 0

namespace Tracer {

struct Tracer {

    Tracer(const char* function, const char* file, uint32_t line);
    Tracer(const char* function, const char* signature, const char* file, uint32_t line);
    Tracer();
    ~Tracer();

    static uint32_t depth();

    void indent(bool showDuration, bool showCounter);
    void functionName(bool call);

    template <typename T>
    void print(T p)
    {
        Serial.print(p);
    }

    template <class... Args>
    void printf(const char* f, Args&&... args)
    {
        Serial.printf(f, std::forward<Args>(args)...);
    }

    String m_function;
    String m_file { 0 };
    uint32_t m_line { 0 };
    uint32_t m_millis { 0 };
    uint32_t m_counter;

    static std::map<TaskHandle_t, uint32_t> taskDepth;
    static uint32_t counter;
};

extern SemaphoreHandle_t syncMutex;

} // namespace Tracer

#define TraceFunction \
    Tracer::Tracer __t(__PRETTY_FUNCTION__, __FILE__, __LINE__);

#define TracePrint(x)                                          \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.print(x);                                              \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TracePrintln(x)                                        \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.print(x);                                              \
    Serial.println();                                          \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TracePrintf(f, ...)                                    \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.printf(f, __VA_ARGS__);                                \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceIPrint(x)                                         \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.indent(true, true);                                    \
    __t.print(x);                                              \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceIPrintln(x)                                       \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.indent(true, true);                                    \
    __t.print(x);                                              \
    Serial.println();                                          \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceIPrintf(f, ...)                                   \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    __t.indent(true, true);                                    \
    __t.printf(f, __VA_ARGS__);                                \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceIPos                                                                             \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY);                                \
    __t.indent(true, true);                                                                   \
    Serial.printf("%s [%u] @ %s:%u", __PRETTY_FUNCTION__, __t.m_counter, __FILE__, __LINE__); \
    Serial.println();                                                                         \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

// Trace functions without a preceeding TraceFunction
#define TraceNPrint(x)                                         \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.print(x);                                              \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNPrintln(x)                                       \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.print(x);                                              \
    Serial.println();                                          \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNPrintf(f, ...)                                   \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.printf(f, __VA_ARGS__);                                \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNIPrint(x)                                        \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.indent(true, true);                                    \
    __t.print(x);                                              \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNIPrintln(x)                                      \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.indent(true, true);                                    \
    __t.print(x);                                              \
    Serial.println();                                          \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNIPrintf(f, ...)                                  \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY); \
    Tracer::Tracer __t;                                        \
    __t.indent(true, true);                                    \
    __t.printf(f, __VA_ARGS__);                                \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#define TraceNIPos                                                        \
    xSemaphoreTakeRecursive(Tracer::syncMutex, portMAX_DELAY);            \
    Tracer::Tracer __t;                                                   \
    __t.indent(true, true);                                               \
    Serial.printf("%s @ %s:%u", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    Serial.println();                                                     \
    xSemaphoreGiveRecursive(Tracer::syncMutex);

#else

#define TraceFunction

#define TracePrint(x)
#define TracePrintln(x)
#define TracePrintf(f, ...)

#define TraceNPrint(x)
#define TraceNPrintln(x)
#define TraceNPrintf(f, ...)

#define TraceIPrint(x)
#define TraceIPrintln(x)
#define TraceIPrintf(f, ...)
#define TraceIPos

#define TraceNIPrint(x)
#define TraceNIPrintln(x)
#define TraceNIPrintf(f, ...)
#define TraceNIPos

#endif
