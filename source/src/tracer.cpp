#include "tracer.h"

#if TRACER > 0

namespace Tracer {

std::map<TaskHandle_t, uint32_t> Tracer::taskDepth;
SemaphoreHandle_t syncMutex = xSemaphoreCreateRecursiveMutex();

Tracer::Tracer(const char* function, const char* file, uint32_t line)
    : m_function(function)
    , m_file(file)
    , m_line(line)
{
    xSemaphoreTakeRecursive(syncMutex, portMAX_DELAY);
    m_millis = millis();
    indent(false);
    functionName(true);

    auto& d(taskDepth[xTaskGetCurrentTaskHandle()]);
    ++d;
    xSemaphoreGiveRecursive(syncMutex);
}

Tracer::Tracer()
{
    xSemaphoreTakeRecursive(syncMutex, portMAX_DELAY);
    xSemaphoreGiveRecursive(syncMutex);
}

Tracer::~Tracer()
{
    xSemaphoreTakeRecursive(syncMutex, portMAX_DELAY);
    if (!m_function.isEmpty()) {
        auto& d(taskDepth[xTaskGetCurrentTaskHandle()]);
        --d;
        indent(true);
        functionName(false);
    }
    xSemaphoreGiveRecursive(syncMutex);
}

void Tracer::indent(bool duration)
{
    String prefix("[");
    prefix += String(millis() / 1000);
    prefix += "] ";
    if (m_millis != 0 && duration) {
        prefix += "[";
        prefix += String((millis() - m_millis) / 1000);
        prefix += "] ";
    }
    while (prefix.length() < 12) {
        prefix += " ";
    }
    Serial.print(prefix);
    Serial.printf("%17s", pcTaskGetTaskName(nullptr));
    Serial.print("@");
    Serial.print(xPortGetCoreID());
    Serial.print(" ");
    auto d = depth();
    for (int i = 0; i < d; ++i) {
        Serial.print("|   ");
    }
}

void Tracer::functionName(bool call)
{
    if (call) {
        Serial.print("> ");
    } else {
        Serial.print("< ");
    }
    Serial.print(m_function);
    Serial.print(" @ ");
    Serial.print(m_file);
    if (call) {
        Serial.print(":");
        Serial.print(m_line);
    }
    Serial.println();
}

uint32_t Tracer::depth()
{
    return taskDepth[xTaskGetCurrentTaskHandle()];
}

} // namespace Tracer

#else

#endif
