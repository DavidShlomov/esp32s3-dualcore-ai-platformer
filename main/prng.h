#pragma once

#include <stdint.h>
#include "esp_random.h"

/// @brief Generates a random number with xorshift.
/// @return Random number between 0 - UINT32_MAX;
uint32_t get_random_num(void);