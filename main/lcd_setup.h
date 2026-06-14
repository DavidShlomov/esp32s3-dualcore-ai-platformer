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
#include "lcd_setup.h"

esp_lcd_panel_handle_t lcd_setup_main();
void screen_config(spi_bus_config_t *screen);
void io_spi_config(esp_lcd_panel_io_spi_config_t *io_config);
void panel_config(esp_lcd_panel_dev_config_t *panel_dev);
void draw_rect(esp_lcd_panel_handle_t panel_handle);
void black_screen(esp_lcd_panel_handle_t panel_handle);