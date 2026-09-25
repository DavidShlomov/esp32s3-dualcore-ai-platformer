#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"    // Required for SPI bus and DMA initialization
#include "esp_lcd_panel_io.h"     // Required for LCD panel IO operations
#include "esp_lcd_panel_vendor.h" // Required for specific panel drivers (like ST7789)
#include "esp_lcd_panel_ops.h"    // Required for drawing commands
#include "driver/gpio.h"          // Required for general GPIO control (e.g., Backlight)
#include "esp_lcd_ili9341.h"      //lcd driver.
#include <math.h>

/// @brief Initializes the SPI bus, LCD IO layer, and ILI9341 panel, then clears the screen and returns the ready-to-use panel handle.
/// @param my_callback Callback invoked when an SPI DMA color transfer completes (see on_color_trans_done() in main.c).
/// @return Handle to the initialized ILI9341 panel, ready for esp_lcd_panel_draw_bitmap() calls.
esp_lcd_panel_handle_t
lcd_setup_main(esp_lcd_panel_io_color_trans_done_cb_t my_callback);

/// @brief Fills in the SPI bus configuration for the LCD (MOSI = GPIO15, SCLK = GPIO16, no MISO/quad lines).
/// @param screen SPI bus config struct to populate.
void screen_config(spi_bus_config_t *screen);

/// @brief Fills in the LCD panel IO configuration over SPI (CS = GPIO5, DC = GPIO7, 40MHz clock, mode 0).
/// @param io_config Panel IO config struct to populate.
/// @param my_callback Callback registered to run when a color transfer completes.
void io_spi_config(esp_lcd_panel_io_spi_config_t *io_config, esp_lcd_panel_io_color_trans_done_cb_t my_callback);

/// @brief Fills in the ILI9341 panel device configuration (reset = GPIO6, RGB order, 16 bits per pixel).
/// @param panel_dev Panel device config struct to populate.
void panel_config(esp_lcd_panel_dev_config_t *panel_dev);

/// @brief Fills the entire screen in row chunks to keep the temporary buffer small.
/// @param panel_handle Screen panel handle.
void black_screen(esp_lcd_panel_handle_t panel_handle);