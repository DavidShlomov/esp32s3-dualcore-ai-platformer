#include "game.h"
#include "configs.h"
#include "prng.h"
#include <stdio.h>
#include <stdlib.h>


//generate random map array.
uint8_t *build_map()
{
    uint8_t *map_array = calloc(MAP_SIZE, sizeof(uint8_t));

    if(map_array == NULL){
        fprintf(stderr, "Fatal Error: map_array memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    //starting with 12 empty cells, at least 2 last tiles empty.
    for (int i = NUM_OF_TILES_IN_SCREEN; i < MAP_SIZE - 2; i++)
    {
        if (get_random_num() % 2 == 0)
        {
            map_array[i] = get_random_num() % 90 + 40;
            i += 3;
        }
    }

    return map_array;
}


void check_collision(int character_global_x, int character_y_axis, uint8_t *curr_level, bool *is_dead){
    // Calculate the global X of the character's right and left edges
    int char_right_edge = character_global_x + CIRCLE_CHARACTER_RADIUS;
    int char_left_edge = character_global_x - CIRCLE_CHARACTER_RADIUS;
    int char_bottom_edge = character_y_axis - CIRCLE_CHARACTER_RADIUS;

    // Find the range of tiles the character is currently overlapping (Left and Right)
    int start_index = char_left_edge / OBSTACLE_WIDTH;
    int end_index = char_right_edge / OBSTACLE_WIDTH;

    // Loop through all tiles the character touches to prevent clipping through corners
    for (int idx = start_index; idx <= end_index; idx++)
    {
        // Check if there is an actual obstacle in this specific index
        if (idx >= 0 && idx < MAP_SIZE && curr_level[idx] != 0)
        {
            // Extract the obstacle's parameters
            int obs_left_edge = idx * OBSTACLE_WIDTH;
            int obs_right_edge = obs_left_edge + OBSTACLE_WIDTH;
            int obs_top_edge = ROAD_HEIGHT + curr_level[idx];

            // Mathematical overlap check
            if (char_right_edge > obs_left_edge && char_left_edge < obs_right_edge && char_bottom_edge < obs_top_edge)
            {
                *is_dead = true;
                break; // Once we hit something, no need to check other tiles
            }
        }
    }
}


//Agent jumping logic need to callibrate numbers later.
void agent_jump(Agent *agent, int *y_velocity, int dna_array_index, int *agent_y_axis){
    int gravity = 1;

    if(dna_array_index >= DNA_SIZE){
        printf("dna_array_index is out of bound.\n");
        return;
    }
    //Agent is currenly not mid jump. 
    if(*y_velocity == 0 && *agent_y_axis == CIRCLE_CHARACTER_INITIAL_Y){
        switch (agent->dna[dna_array_index])
        {
            case LOW:
                *y_velocity = 16;
                break;
            case MEDIUM:
                *y_velocity = 18;
                break;
            case HIGH:
                *y_velocity = 20;
                break;
            default:
                break;
        }
    }
    //Physics
    *y_velocity -= gravity;
    *agent_y_axis += *y_velocity;
    //Physics corretion
    if(*agent_y_axis <= CIRCLE_CHARACTER_INITIAL_Y){
        *y_velocity = 0;
        *agent_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    }
}


void calculate_fitness(int *fitness, int agent_y_axis, int map_index, bool *above_obstacle, uint8_t *curr_level, bool *received_map_pass_points){

    //agent is above an obstacle.
    if(agent_y_axis > CIRCLE_CHARACTER_INITIAL_Y && curr_level[map_index] > 0){
        *above_obstacle = true;
    }
    //agent passed obstacle successfully.
    if(*above_obstacle && agent_y_axis == CIRCLE_CHARACTER_INITIAL_Y){
        *fitness += 100;
        *above_obstacle = false;
    }
    //airtime fitmess is -1.
    if(agent_y_axis > CIRCLE_CHARACTER_INITIAL_Y){
        *fitness -= 1;
    }
    //last tile guranteed with no obstacle, map is passed, not getting multiple level passing points.
    if(map_index == MAP_SIZE - 1 && !*received_map_pass_points){
        *fitness += 1000;
        *received_map_pass_points = true;
    }
}

void print_map_array(const uint8_t *map_array)
{
    for (int i = 0; i < MAP_SIZE; i++)
    {
        printf("%d ", map_array[i]);
    }
    printf("\n");
}
