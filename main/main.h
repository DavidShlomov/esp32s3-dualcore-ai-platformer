#pragma once

#include <stdint.h>
#include "freertos/semphr.h"
#include <stdbool.h>
#include "esp_lcd_panel_io.h"

#define PURPLE_COLOR __builtin_bswap16(0x200b)
#define BURGUNDY_COLOR __builtin_bswap16(0x8000)
#define BLACK_COLOR 0x0000
#define WHITE_COLOR 0xffff
#define ROAD_HEIGHT 10
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define ROAD_SIZE (ROAD_HEIGHT * SCREEN_WIDTH)
#define SCREEN_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH)
#define OBSTACLE_WIDTH 30
#define TRANSPARENT_COLOR 0x54a2
#define CIRCLE_CHARACTER_X_AXIS 130
#define CIRCLE_CHARACTER_RADIUS 10
#define CIRCLE_CHARACTER_INITIAL_Y (CIRCLE_CHARACTER_RADIUS + ROAD_HEIGHT)
#define MAP_SIZE 100
#define DNA_SIZE MAP_SIZE
#define NUM_OF_TILES_IN_SCREEN 12
#define GENERATION_SIZE 50
#define CHAMPION_SIZE 10
#define NUM_OF_GENERATIONS 2500
#define MOVE_RATE 3

/* WILL NEED THIS LATER  WHEN AGENT IS CALC IN ANOTHER FILE*/
// extern uint32_t random_num;
// extern SemaphoreHandle_t dma_ready_sem;
// extern int camera_x;

enum jump_strength
{
    NONE,
    LOW,
    MEDIUM,
    HIGH
};

typedef struct
{
    uint8_t dna[DNA_SIZE]; // 0 = do nothing, 1 = jump low, 2 = jump medium, 3 = jump high
    int fitness; // The score of the agent hit an obstacle.
    bool is_dead; // True if agent hit an obstacle. 
    int distance; // How many tiles the agent successfully passed.
    uint8_t *map;// The map that the agent successfully passed, assigned only on the champion.
}Agent;

/// @brief Draws the road into the screen's PSRAM buffer.
/// @param screen_buffer Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements.
void draw_road(uint16_t *screen_buffer);

/// @brief Draws a rectangular obstacle into the screen's PSRAM buffer.
/// @param screen_buffer Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements.
/// @param height Obstacle height in pixels.
/// @param width Width of the obstacle in pixels.
/// @param x_axis Obstacle's leftmost X coordinate, in screen pixels (0 = left edge).
/// @param y_axis Y start position of the obstacle, measured upward from the road's bottom-most edge.
void create_obstacle(uint16_t *screen_buffer, int height, int width, int x_axis, int y_axis);

/// @brief Seeds the PRNG state from the hardware RNG, guaranteeing a non-zero seed.
/// @note Sets the global `random_num` value used by xorshift32().
void init_xorshift32(void);

/// @brief Advances the PRNG state one step using the xorshift32 algorithm.
/// @note Mutates the global `random_num` value in place. init_xorshift32() must be called first to seed it.
void xorshift32(void);

/// @brief ISR callback invoked by the LCD driver when an SPI DMA color transfer completes.
/// @param panel_io Handle of the LCD panel IO layer that completed the transfer (unused).
/// @param edata Event data for the completed transaction (unused).
/// @param user_ctx User context pointer registered with the callback (unused).
/// @return true if giving the semaphore woke a higher-priority task, requesting an immediate context switch.
/// @note Runs in interrupt context (IRAM_ATTR). Signals `dma_ready_sem` so app_main() may reuse the buffer that was just sent to the display.
bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

/// @brief Allocates a new full-screen pixel buffer in PSRAM.
/// @note Never returns NULL — halts the calling task indefinitely if allocation fails.
/// @return Pointer to a newly allocated buffer of SCREEN_SIZE uint16_t elements in PSRAM.
uint16_t *allocate_screen_buffer(void);

/// @brief Fills the entire screen buffer with black.
/// @param screen_buffer Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements.
void fill_screen_black(uint16_t *screen_buffer);

/// @brief Writes a buffer to the display panel via SPI DMA.
/// @param panel_handle Screen panel handle.
/// @param screen_buffer Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements.
/// @note Non-blocking: the transfer runs asynchronously and completion is signaled via on_color_trans_done().
void flush_screen_buffer(esp_lcd_panel_handle_t panel_handle, uint16_t *screen_buffer);

/// @brief Draws a circular shaped character into the screen's PSRAM buffer.
/// @param screen_buffer Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements.
/// @param x_start Center of the circle x coordinate on screen.
/// @param y_start Center of the circle y coordinate on screen.
/// @param radius Radius of the character in pixels (decides the size of the character).
void draw_circle(uint16_t *screen_buffer, int x_start, int y_start, int radius);

/// @brief Generates a random obstacle map, where each byte represents an obstacle's height (0 = no obstacle).
/// @return Pointer to a heap-allocated array of MAP_SIZE uint8_t elements.
/// @note Caller takes ownership of the returned array and is responsible for freeing it.
uint8_t *build_map(void);

/// @brief Prints the level's obstacle-height array to stdout, space-separated.
/// @param map_array Pointer to an array of MAP_SIZE uint8_t elements (see build_map()).
void print_map_array(const uint8_t *map_array);


/// @brief Draws the "LEVEL END" screen and blocks for ~3 seconds before returning.
/// @param panel_handle Screen panel handle.
void show_level_end_screen(esp_lcd_panel_handle_t panel_handle);


/// @brief Draws obstacle map based on map array given.
/// @param map_index Current map index in the array.
/// @param map_offset Pixel offset within the current tile (camera_x % OBSTACLE_WIDTH), used for smooth scrolling.
/// @param y_start_pos Y start position of the obstacle, measured upward from the road's bottom-most edge.
/// @param curr_level Array of obstacles of the current level.
/// @param screen_buffer Final Screen PSRAM array, must point to a buffer of at least SCREEN_SIZE uint16_t elements. 
void draw_visible_obstacles(int map_index, int map_offset, int y_start_pos ,uint8_t *curr_level, uint16_t *screen_buffer);


/// @brief Checks if the character has collided with an obstacle.
/// @param character_global_x Character location on the global map.
/// @param character_y_axis Character location on the y axis.
/// @param curr_level Level array of obstacles.
/// @param is_dead Output parameter set to true if the character overlaps an obstacle.
void check_collision(int character_global_x, int character_y_axis, uint8_t *curr_level, bool *is_dead);

/// @brief Checks if character died or map ended.
/// @param map_index Current map index in the array.  
/// @param character_y_axis Character center y axis location on screen, gets rewritten.
/// @param is_dead True if a collision occured, false otherwise, gets rewritten.
/// @param y_velocity Y velocity of the jump, gets rewritten.
/// @param panel_handle Screen panel handle.
/// @note If game over, blocks 500ms then calls show_level_end_screen() which blocks a further 3 seconds.
void check_game_over(int map_index, int *character_y_axis, bool *is_dead, int *y_velocity,esp_lcd_panel_handle_t panel_handle);

/// @brief Initiates a single game agent 
/// @param new_agent Current agent to initiate.
/// @note Resets an agent's fitness/distance/is_dead to defualts and randomizes its DNA. Does not touch agent-> map.
void init_agent(Agent *new_agent);

/// @brief Randomizes agent initial DNA .
/// @param new_agent Current agent to randomize DNA.
/// @note Agent DNA is same length as MAP_SIZE.
/// @note Advances the global random_num PRNG state via repeated xorshift() calls.
void randomize_agent_dna(Agent *new_agent);

/// @brief Prints all agent parameters for debugging.
/// @param agent Current agent to print parameters of.
void print_agent(Agent *agent);

/// @brief Simulates a headless agent run without rendering.  
/// @param agent Current agent to simulate.
/// @param curr_level Current level agents are training on.
/// @note Precondition: agent->dna and agent-> map must already be initialized (see init_agent()) before calling.
void simulate_agent(Agent *agent, uint8_t *curr_level);

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

/// @brief Allocates agent generation dynamically.
/// @param array_size Agent array size.
/// @return An empty array of agent type sizeof(array_size), NULL if failes, caller responsible of freeing.
Agent *allocate_agent_array(int array_size);

/// @brief Initiate agent generarion of size array_size.
/// @param array_size Agent array size.
void init_agent_array(Agent *agent_generation, int array_size);

/// @brief prints a full generation of agents sizeof(GENERATION_SIZE).
/// @param agent_generation Generation to be printed.
/// @param array_size Agent array size.
void print_agent_array(Agent *agent_generation, int array_size);

/// @brief Allocating champion agent array using calloc for reset.
/// @param array_size Champions array size.
/// @return Champion array sizeof(array_size), caller responsible to free.
Agent *allocate_champion_array(int array_size);

/// @brief Simulates a full generation of agents.
/// @param agent_generation Current generation to simulate.
/// @param current_level Current level to simulate.
/// @param simulation_index_start the index of the agent array to start running simulations from.
void simulate_agent_generation(Agent *agent_generation, uint8_t *current_level, int simulation_index_start);

/// @brief Fitness compare function for qsort.
/// @param agent_a First agent to compare.
/// @param agent_b Second agent to compare.
/// @return Difference between agent_b fitness and agent_a fitness for decending fitness.
int compare_fitness(const void *agent_a, const void *agent_b);

/// @brief Implements natural selection mechanism from biology.
/// @param agent_generation Agent array to train.
/// @param map_level Current map the agents are training on.
/// @note terminates after NUM_OF_GENERATIONS.
void natural_selection(Agent *agent_generation, uint8_t *map_level);

/// @brief Implements cross-over mechanism from biology.
/// @param parent_a_dna first parent dna array.
/// @param parent_b_dna second parent dna array.
/// @param child_dna child dna to be rewritten.
void double_cross_over(uint8_t *parent_a_dna, uint8_t *parent_b_dna, uint8_t *child_dna);

/// @brief Implementing mutation mechanism from biology.
/// @param child_dna child dna array.
/// @param mutation_rate the percent at which the mutation occurs.
void mutation(uint8_t *child_dna, float mutation_rate_percent);

/// @brief Resets agent is_dead, fitness, distance parameters for re-simulating.
/// @param agent_generation Agent generation to reset.
/// @param generation_index_start Index in which to start the reset of the current generation.
void generation_reset(Agent *agent_generation, int generation_index_start);