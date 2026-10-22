#include "lcd_setup.h"
#include <stdlib.h>
#include "game.h"
#include "configs.h"
#include "render.h"
#include "genetic_algorithm.h"
#include "freertos/semphr.h"

static SemaphoreHandle_t dma_ready_sem = NULL;

// Main entry point for the game engine
void app_main(void)
{
    uint16_t *front_buffer = allocate_screen_buffer();
    uint16_t *back_buffer = allocate_screen_buffer();
    uint16_t *temp_buffer;
    dma_ready_sem = xSemaphoreCreateBinary();
    esp_lcd_panel_handle_t panel_handle = lcd_setup_main(on_color_trans_done, dma_ready_sem);
    int y_start_pos = 0;
    int character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    int map_index = 0;
    int map_offset = 0;
    int y_velocity = 0;
    int camera_x = 0;
    uint8_t *map_level;
    Agent *agent_generation = NULL;
    Agent *champion_agent_array = NULL;//TODO: later add a champion array sized 10.

    if(dma_ready_sem == NULL){
        fprintf(stderr, "Fatal Error: semaphore creation failed.\n");
        abort();
    }

    xSemaphoreGive(dma_ready_sem);

    // Print initialization message
    printf("ESP32-S3 Game Engine Initialized!\n");

    map_level = build_map();

    agent_generation = allocate_agent_array(GENERATION_SIZE);
    init_agent_array(agent_generation, GENERATION_SIZE);

    simulate_agent_generation(agent_generation, map_level, 0);

    qsort(agent_generation, GENERATION_SIZE, sizeof(Agent), compare_fitness);


    print_agent(&agent_generation[0]);
    print_map_array(map_level);

    natural_selection(agent_generation, map_level);
    print_agent(&agent_generation[0]);


    // Infinite loop to keep the task alive
    while (1)
    {
        //game tiles are obst width for simplicity.
        map_index = camera_x / OBSTACLE_WIDTH;
        map_offset = camera_x % OBSTACLE_WIDTH;

        // Calculate where the character is in the global map
        
        int character_global_x = camera_x + CIRCLE_CHARACTER_X_AXIS;
        int character_map_index = character_global_x / OBSTACLE_WIDTH;

        if(character_map_index >= DNA_SIZE){
            character_map_index = DNA_SIZE - 1;
        }

        xSemaphoreTake(dma_ready_sem, portMAX_DELAY);
        fill_screen_black(back_buffer);
        draw_road(back_buffer);

        draw_visible_obstacles(map_index, map_offset, y_start_pos, map_level, back_buffer);

        agent_jump(&agent_generation[0], &y_velocity, character_map_index, &character_y_axis);

        draw_circle(back_buffer, CIRCLE_CHARACTER_X_AXIS, character_y_axis, CIRCLE_CHARACTER_RADIUS);

        check_collision(character_global_x,character_y_axis, map_level, &agent_generation[0].is_dead);

        temp_buffer = front_buffer;
        front_buffer = back_buffer;
        back_buffer = temp_buffer;

        flush_screen_buffer(panel_handle, front_buffer);
        
        camera_x += MOVE_RATE;
        // check_game_over may reset camera_x to 0, so the increment must come first
        check_game_over(map_index, &character_y_axis, &agent_generation[0].is_dead, &y_velocity , &camera_x, panel_handle);

    }
}


/*


            ****TO DO****
            Training time is ~50 sec, run time is  ~40 need optimization to be less training time then running for smooth level transition.

            
            ****SCALING****
            make consectual agents running
            mid run new map agent training, running in succesion.
            split into 2 cores.


            ****BONUS****
            calculate actual jump peak of velocity to make the highest option go exactly over the highest obctacle.
*/