#include <stdio.h>
#include <stdlib.h>
#include "configs.h"
#include "game.h"
#include "esp_random.h"
#include <time.h>
#include "genetic_algorithm.h"
#include <string.h>


static double now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <seed>\n", argv[0]);
		return 1;
	}

	g_stub_seed = (uint32_t)strtoul(argv[1], NULL, 10);

	uint8_t training_map[MAP_SIZE];
	build_map(training_map);
	int num_of_obstacles = 0;

	for(int i = 0; i < MAP_SIZE; i++){
		if(training_map[i] != 0){
			num_of_obstacles++;
		}
	}

	Agent *generation = allocate_agent_array(GENERATION_SIZE);
	Agent champion;


	double start = now_ms();
	init_agent_array(generation, GENERATION_SIZE);
	simulate_agent_generation(generation, training_map, 0);
	TrainingStats training_stats = natural_selection(generation, training_map);
	double train_ms = now_ms() - start;

	copy_agent_param(&generation[0], &champion);
	
	int map_ok = (memcmp(training_map, champion.map, MAP_SIZE) == 0);

	printf("RESULT seed=%u fitness=%d fitness_diff=%d distance=%d dead=%d first_pass_gen=%d total_gen=%d obstacles=%d train_ms=%.1f map_ok=%d\n",
		 g_stub_seed, champion.fitness, training_stats.fitness_diff, champion.distance, champion.is_dead,
		 training_stats.first_generation_pass,training_stats.total_num_of_generations, num_of_obstacles,train_ms,
		 map_ok);

	free(generation);

	return 0;
}
