#pragma once

#include <stdint.h>


/// @brief Generates a random number with xorshift.
/// @return Random number between 0 - UINT32_MAX;
/// @note Thread safe, not worth it in cpu cycle to mutex.
uint32_t get_random_num(void);