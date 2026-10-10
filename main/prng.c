#include "prng.h"
static uint32_t random_num;


//randomizer
static void xorshift32()
{
    random_num ^= random_num << 13;
    random_num ^= random_num >> 17;
    random_num ^= random_num << 5;
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

    if(random_num == 0){
        init_xorshift32();
    }
    xorshift32();
    return random_num;
}