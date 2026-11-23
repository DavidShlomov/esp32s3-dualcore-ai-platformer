#pragma once

#include <stdint.h>
#include "configs.h"
#include <stdbool.h>

typedef struct
{
    uint8_t dna[DNA_SIZE]; // 0 = do nothing, 1 = jump low, 2 = jump medium, 3 = jump high
    int fitness; // The score of the agent hit an obstacle.
    bool is_dead; // True if agent hit an obstacle. 
    int distance; // How many tiles the agent successfully passed.
    uint8_t map[MAP_SIZE];// The map that the agent trained on, assigned only on the champion.
}Agent;

/// @brief stats of one natural selection run.
typedef struct
{
    int total_num_of_generations; // total number of generations agent_array ran.
    int first_generation_pass; // first generation that the agent array passed the level, 0 if passed before evolution, -1 if none passed.
    int fitness_diff; // fitness difference between the first passing agent to the best champion found, -1 if none passed.
}TrainingStats;


/// @brief Simulates a headless agent run without rendering.  
/// @param agent Current agent to simulate.
/// @param curr_level Current level agents are training on.
/// @note Precondition: agent->dna and agent-> map must already be initialized (see init_agent()) before calling.
void simulate_agent(Agent *agent, const uint8_t *curr_level);


/// @brief Simulates a full generation of agents.
/// @param agent_generation Current generation to simulate.
/// @param curr_level Current level to simulate.
/// @param simulation_index_start the index of the agent array to start running simulations from.
void simulate_agent_generation(Agent *agent_generation, const uint8_t *curr_level, int simulation_index_start);

/// @brief Fitness compare function for qsort.
/// @param agent_a First agent to compare.
/// @param agent_b Second agent to compare.
/// @return Difference between agent_b fitness and agent_a fitness for decending fitness.
int compare_fitness(const void *agent_a, const void *agent_b);

/// @brief Implements natural selection mechanism from biology.
/// @param agent_generation Agent array to train.
/// @param map_level Current map the agents are training on.
/// @note Terminates after NUM_OF_GENERATIONS or fitness of agent_generation[0] is above 2100.
/// @note Best agent is in agent_generation[0].
/// @note Best agent->map field contains the map level array that he trained on.
/// @return Returns a struct of training stats of the champion agent.
TrainingStats natural_selection(Agent *agent_generation, uint8_t *map_level);

/// @brief Implements cross-over mechanism from biology.
/// @param parent_a_dna first parent dna array.
/// @param parent_b_dna second parent dna array.
/// @param child_dna child dna to be rewritten.
void double_cross_over(uint8_t *parent_a_dna, uint8_t *parent_b_dna, uint8_t *child_dna);

/// @brief Implementing mutation mechanism from biology.
/// @param child_dna child dna array.
/// @param mutation_rate_percent the percent at which the mutation occurs.
void mutation(uint8_t *child_dna, float mutation_rate_percent);

/// @brief Initiates a single game agent 
/// @param new_agent Current agent to initiate.
/// @note Resets an agent's fitness/distance/is_dead to defualts and randomizes its DNA. Does not touch agent-> map.
void init_agent(Agent *new_agent);

/// @brief Randomizes agent initial DNA .
/// @param new_agent Current agent to randomize DNA.
/// @note Agent DNA is same length as MAP_SIZE.
void randomize_agent_dna(Agent *new_agent);

/// @brief Prints all agent parameters for debugging.
/// @param agent Current agent to print parameters of.
void print_agent(const Agent *agent);

/// @brief Allocates agent generation dynamically.
/// @param array_size Agent array size.
/// @return An empty array of agent type sizeof(array_size), NULL if failes, caller responsible of freeing.
Agent *allocate_agent_array(int array_size);

/// @brief Initiate agent generarion of size array_size.
/// @param agent_generation agent array pointer to init.
/// @param array_size Agent array size.
void init_agent_array(Agent *agent_generation, int array_size);

/// @brief prints a full generation of agents sizeof(GENERATION_SIZE).
/// @param agent_generation Generation to be printed.
/// @param array_size Agent array size.
void print_agent_array(const Agent *agent_generation, int array_size);

/// @brief Allocating champion agent array using calloc for reset.
/// @param array_size Champions array size.
/// @return Champion array sizeof(array_size), caller responsible to free.
Agent *allocate_champion_array(int array_size);

/// @brief Resets agent is_dead, fitness, distance parameters for re-simulating.
/// @param agent_generation Agent generation to reset.
/// @param generation_index_start Index in which to start the reset of the current generation.
void generation_reset(Agent *agent_generation, int generation_index_start);

/// @brief Copies agent parameters from one agent to another. 
/// @param source Agent to copy from.
/// @param destination Agent to copy to.
/// @note Copies all agent parameters.
void copy_agent_param(Agent *source, Agent *destination);
