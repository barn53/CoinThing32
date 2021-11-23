#pragma once
#include "tracer.h"
#include <Arduino.h>

namespace cointhing {

enum class NumberFormat : uint8_t {
    THOUSAND_DOT_DECIMAL_COMMA = 0, // 1.000,00
    THOUSAND_BLANK_DECIMAL_COMMA, // 1 000,00
    DECIMAL_COMMA, // 1000,00
    THOUSAND_COMMA_DECIMAL_DOT, // 1,000.00
    THOUSAND_BLANK_DECIMAL_DOT, // 1 000.00
    DECIMAL_DOT // 1000.00
};
void formatNumber(float n, String& s, NumberFormat format, bool forceSign, bool dash00, uint8_t forceDecimalPlaces = std::numeric_limits<uint8_t>::max());

String timeFromTimestamp(time_t timestamp);

class MutexGuard_ {
public:
    MutexGuard_(SemaphoreHandle_t mutex, const char* name, const char* file, const char* function, uint32_t line)
        : m_guard(mutex)
        , m_name(name)
        , m_function(function)
    {
        xSemaphoreTake(m_guard, portMAX_DELAY);
        ++counter;
        m_counter = counter;
        TraceNIPrintf("+ take mutex %s[%u] @ %s() @ %s:%u\n", m_name, m_counter, m_function, file, line);
    }
    ~MutexGuard_()
    {
        TraceNIPrintf("- give mutex %s[%u] @ %s()\n", m_name, m_counter, m_function);
        xSemaphoreGive(m_guard);
    }

private:
    SemaphoreHandle_t m_guard;
    const char* m_name;
    const char* m_function;
    uint32_t m_counter;
    static uint32_t counter;
};

class RecursiveMutexGuard_ {
public:
    RecursiveMutexGuard_(SemaphoreHandle_t recursiveMutex, const char* name, const char* function, const char* file, uint32_t line)
        : m_guard(recursiveMutex)
        , m_name(name)
        , m_function(function)
    {
        xSemaphoreTakeRecursive(m_guard, portMAX_DELAY);
        ++counter;
        m_counter = counter;
        TraceNIPrintf("+ take recursive mutex %s[%u] @ %s() @ %s:%u\n", m_name, m_counter, m_function, file, line);
    }
    ~RecursiveMutexGuard_()
    {
        TraceNIPrintf("- give recursive mutex %s[%u] @ %s()\n", m_name, m_counter, m_function);
        xSemaphoreGiveRecursive(m_guard);
    }

private:
    SemaphoreHandle_t m_guard;
    const char* m_name;
    const char* m_function;
    uint32_t m_counter;
    static uint32_t counter;
};

#define MutexGuard(m) \
    MutexGuard_ __g_##m(m, #m);

#define RecursiveMutexGuard(m) \
    RecursiveMutexGuard_ __g_##m(m, #m, __FUNCTION__, __FILE__, __LINE__);

} // namespace cointhing
