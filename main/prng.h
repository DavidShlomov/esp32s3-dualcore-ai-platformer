#pragma once

#include <stdint.h>


/// @brief Generates a random number with xorshift.
/// @return Random number between 0 - UINT32_MAX;
/// @note Not thread safe, will be used only on core 1.
uint32_t get_random_num(void);