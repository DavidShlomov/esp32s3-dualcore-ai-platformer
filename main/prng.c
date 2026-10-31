#include "prng.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "esp_random.h"
#include "freertos/semphr.h"

static uint32_t random_num = 0;

//randomizer
static uint32_t xorshift32()
{
    random_num ^= random_num << 13;
    random_num ^= random_num >> 17;
    random_num ^= random_num << 5;

    return random_num;
}

static void init_xorshift32()
{
    random_num = esp_random();

    if (random_num == 0)
    {
        random_num = 1;
    }
}

uint32_t get_random_num(){
    uint32_t random = 0;

    if(random_num == 0){
        init_xorshift32();
    }
    random = xorshift32();
    
    return random;
}