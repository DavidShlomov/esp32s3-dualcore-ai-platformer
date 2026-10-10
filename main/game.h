#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "genetic_algorithm.h"



/// @brief Generates a random obstacle map, where each byte represents an obstacle's height (0 = no obstacle).
/// @return Pointer to a heap-allocated array of MAP_SIZE uint8_t elements.
/// @note Caller takes ownership of the returned array and is responsible for freeing it.
uint8_t *build_map(void);

/// @brief Checks if the character has collided with an obstacle.
/// @param character_global_x Character location on the global map.
/// @param character_y_axis Character location on the y axis.
/// @param curr_level Level array of obstacles.
/// @param is_dead Output parameter set to true if the character overlaps an obstacle.
void check_collision(int character_global_x, int character_y_axis, uint8_t *curr_level, bool *is_dead);

/// @brief Agent jumping logic.
/// @param agent Current agent that is run.
/// @param y_velocity Y velocity of the jump, gets rewritten.
/// @param dna_array_index Current agent DNA index of the current tile.
/// @param agent_y_axis Current agent y axis location on the map, gets rewritten.
void agent_jump(Agent *agent, int *y_velocity, int dna_array_index, int *agent_y_axis);

/// @brief Calculates fitness of the agent.
/// @param fitness fitness of the agent, need to be set to 0 (see init_agent()).
/// @param agent_y_axis Current agent y axis location on the map.
/// @param map_index Current map index.
/// @param  above_obstacle true if agent currently above an obstacle, reset to false.  
/// @param curr_level Current level agents are training on.
/// @param received_map_pass_points - Flag if agent passed and received game level pass points. 
/// @note above_obstacle is handeled inside current function, only need to reset a bool to false. 
void calculate_fitness(int *fitness, int agent_y_axis, int map_index, bool *above_obstacle, uint8_t *curr_level, bool *received_map_pass_points);

/// @brief Prints the level's obstacle-height array to stdout, space-separated.
/// @param map_array Pointer to an array of MAP_SIZE uint8_t elements (see build_map()).
void print_map_array(const uint8_t *map_array);