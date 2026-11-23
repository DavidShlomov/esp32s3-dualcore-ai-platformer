#pragma once

#include <stdint.h>

typedef uint32_t TickType_t;

static inline void vTaskDelay(TickType_t ticks)
{ (void)ticks; }
