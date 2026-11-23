#include "esp_random.h"

uint32_t g_stub_seed = 1;

uint32_t esp_random(void)
{
	return g_stub_seed;
}

