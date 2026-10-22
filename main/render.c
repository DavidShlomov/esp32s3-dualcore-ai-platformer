#include "render.h"
#include "esp_lcd_panel_io.h"
#include "esp_heap_caps.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_ops.h"
#include "configs.h"
#include "esp_attr.h"
#include "freertos/task.h"
#include "esp_log.h"


void draw_road(uint16_t *screen_buffer)
{
    for (int i = 0; i < ROAD_SIZE; i++)
    {
        screen_buffer[i] = PURPLE_COLOR;
    }
}

void create_obstacle(uint16_t *screen_buffer, int height, int width, int x_axis_start, int y_axis_start)
{

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int current_x = x_axis_start + x;
            int current_y = ROAD_HEIGHT + y_axis_start + y;

            if (current_x < 0 || current_x >= SCREEN_WIDTH || current_y < ROAD_HEIGHT || current_y >= SCREEN_HEIGHT)
            {
                continue;
            }

            int current_index = (current_y * SCREEN_WIDTH) + current_x;

            screen_buffer[current_index] = WHITE_COLOR;
        }
    }
}

// circle character for simplicity.
void draw_circle(uint16_t *screen_buffer, int x_center, int y_center, int radius)
{
    for (int y = 0; y < 2 * radius; y++)
    {
        for (int x = 0; x < 2 * radius; x++)
        {
            int current_x = x_center - radius + x;
            int current_y = y_center - radius + y;
            int current_index = (current_y * SCREEN_WIDTH) + current_x;


            if (radius * radius >= (x - radius) * (x - radius) + (y - radius) * (y - radius))
            {
                screen_buffer[current_index] = BURGUNDY_COLOR;
            }
        }
    }
}

//init psram buffer.
uint16_t *allocate_screen_buffer()
{
    uint16_t *screen_buffer = (uint16_t *)heap_caps_malloc(SCREEN_SIZE * sizeof(uint16_t), MALLOC_CAP_SPIRAM);

    if (!screen_buffer)
    {
        ESP_LOGE("MEM_TEST", "Fatal Error: screen_buffer memory allocation failed.\n");
        abort();
    }
    return screen_buffer;
}

void fill_screen_black(uint16_t *screen_buffer)
{

    // memset(screen_buffer, INVERTED_WHITE, screen_size * sizeof(uint16_t));
    for (int i = 0; i < SCREEN_SIZE; i++)
    {
        screen_buffer[i] = BLACK_COLOR;
    }
}

void flush_screen_buffer(esp_lcd_panel_handle_t panel_handle, uint16_t *screen_buffer)
{
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 320, 240, screen_buffer);
}

// ISR for SPI DMA transfer complete.
IRAM_ATTR bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    SemaphoreHandle_t sem = (SemaphoreHandle_t)user_ctx;
    xSemaphoreGiveFromISR(sem, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}

//draw LEVEL END on screen.
void show_level_end_screen(esp_lcd_panel_handle_t panel_handle)
{
    // Initialize buffer and paint it black
    uint16_t *end_screen = allocate_screen_buffer();
    fill_screen_black(end_screen);

    // Bitmap matrix for "LEVEL END" (5 rows, 37 columns)
    // 1 = White pixel, 0 = Black background
    const uint8_t text_bitmap[5][37] = {
        {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0},
        {1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0}};

    // Configuration for retro scaling and screen centering
    int scale = 4; // Each '1' in the bitmap becomes a 4x4 pixel square
    int bitmap_width = 37;
    int bitmap_height = 5;

    // Calculate total size of the text on the screen
    int total_width = bitmap_width * scale;
    int total_height = bitmap_height * scale;

    // Calculate starting X and Y to perfectly center the text
    int start_x = (SCREEN_WIDTH - total_width) / 2;
    int start_y = (SCREEN_HEIGHT - total_height) / 2;

    // Render the bitmap into the screen buffer
    for (int row = 0; row < bitmap_height; row++)
    {
        for (int col = 0; col < bitmap_width; col++)
        {
            if (text_bitmap[row][col] == 1)
            {
                // Draw a scaled block for each active pixel in the bitmap
                for (int dy = 0; dy < scale; dy++)
                {
                    for (int dx = 0; dx < scale; dx++)
                    {
                        int screen_x = start_x + (col * scale) + dx;
                        int screen_y = start_y + ((bitmap_height - 1 - row) * scale) + dy;

                        // Prevent drawing out of bounds just in case
                        if (screen_x >= 0 && screen_x < SCREEN_WIDTH && screen_y >= 0 && screen_y < SCREEN_HEIGHT)
                        {
                            int index = (screen_y * SCREEN_WIDTH) + screen_x;
                            end_screen[index] = WHITE_COLOR;
                        }
                    }
                }
            }
        }
    }

    // Send the buffer to the physical display
    flush_screen_buffer(panel_handle, end_screen);

    // Pause the game for 3 seconds to let the player read it
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Free the dynamically allocated memory to prevent memory leaks
    free(end_screen);
}

void draw_visible_obstacles(int map_index, int map_offset, int y_start_pos ,uint8_t *curr_level, uint16_t *screen_buffer){

            //drawing map based on map array heights.
        for (int i = map_index; i < map_index + NUM_OF_TILES_IN_SCREEN; i++)
        {
            if (i == MAP_SIZE)
            {
                break;
            }
            if (curr_level[i] != 0)
            {
                create_obstacle(screen_buffer, curr_level[i], OBSTACLE_WIDTH, ((i - map_index) * OBSTACLE_WIDTH) - map_offset, y_start_pos);
            }
        }
}


void check_game_over(int map_index, int *character_y_axis, bool *is_dead, int *y_velocity , int *camera_x, esp_lcd_panel_handle_t panel_handle){

    //maybe later add different screens for level finish and death.
    if (map_index >= MAP_SIZE || *is_dead)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS); // Give a half second to see the death
        show_level_end_screen(panel_handle);
        *camera_x = 0;
        *character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
        *is_dead = false;
        *y_velocity = 0;
    }
}
