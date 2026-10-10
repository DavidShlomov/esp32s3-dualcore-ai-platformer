#pragma once

#include <stdint.h>
#include "esp_lcd_panel_io.h"
#include <stdbool.h>





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

/// @brief Checks if character died or map ended.
/// @param map_index Current map index in the array.  
/// @param character_y_axis Character center y axis location on screen, gets rewritten.
/// @param is_dead True if a collision occured, false otherwise, gets rewritten.
/// @param y_velocity Y velocity of the jump, gets rewritten.
/// @param camera_x X position of the current screen frame.
/// @param panel_handle Screen panel handle.
/// @note If game over, blocks 500ms then calls show_level_end_screen() which blocks a further 3 seconds.
void check_game_over(int map_index, int *character_y_axis, bool *is_dead, int *y_velocity, int *camera_x, esp_lcd_panel_handle_t panel_handle);

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

/// @brief ISR callback invoked by the LCD driver when an SPI DMA color transfer completes.
/// @param panel_io Handle of the LCD panel IO layer that completed the transfer (unused).
/// @param edata Event data for the completed transaction (unused).
/// @param user_ctx The DMA-ready semaphore handle (SemaphoreHandle_t), registered through lcd_setup_main().
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
