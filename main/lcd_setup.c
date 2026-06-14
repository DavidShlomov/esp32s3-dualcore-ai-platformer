#include "lcd_setup.h"

esp_lcd_panel_handle_t lcd_setup_main()
{
    spi_bus_config_t screen = {0};
    screen_config(&screen);
    spi_bus_initialize(SPI2_HOST, &screen, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t handle_io = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {0};
    io_spi_config(&io_config);
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &handle_io);

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_dev = {0};
    panel_config(&panel_dev);
    esp_lcd_new_panel_ili9341(handle_io, &panel_dev, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    black_screen(panel_handle);
    draw_rect(panel_handle);

    return panel_handle;
}

void screen_config(spi_bus_config_t *screen)
{
    screen->mosi_io_num = 15;
    screen->sclk_io_num = 16;
    screen->miso_io_num = -1;
    screen->quadhd_io_num = -1;
    screen->quadwp_io_num = -1;
    screen->max_transfer_sz = 320 * 240 * 2 + 8;
}

void io_spi_config(esp_lcd_panel_io_spi_config_t *io_config)
{
    io_config->cs_gpio_num = 5;
    io_config->dc_gpio_num = 7;
    io_config->spi_mode = 0;
    io_config->pclk_hz = 1000 * 1000 * 40;
    io_config->trans_queue_depth = 10;
    io_config->lcd_cmd_bits = 8;
    io_config->lcd_param_bits = 8;
}

void panel_config(esp_lcd_panel_dev_config_t *panel_dev)
{
    panel_dev->reset_gpio_num = 6;
    panel_dev->rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_dev->bits_per_pixel = 16;
}

void draw_rect(esp_lcd_panel_handle_t panel_handle)
{
    // Allocate memory for a 100x100 pixel square on the heap
    uint16_t *color_buffer = (uint16_t *)malloc(100 * 100 * sizeof(uint16_t));

    // Fill the buffer with bright green color (RGB565: 0x07E0)
    for (int i = 0; i < 100 * 100; i++)
    {
        color_buffer[i] = __builtin_bswap16(0x07E0);
    }

    // Draw the bitmap on the screen from coordinates (0,0) to (100,100)
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 100, 100, color_buffer);

    // Free the allocated memory to prevent memory leaks
    free(color_buffer);
}

void black_screen(esp_lcd_panel_handle_t panel_handle)
{
    int chunk_lines = 10;
    uint16_t *color_buffer = (uint16_t *)malloc(240 * chunk_lines * sizeof(uint16_t));

    // fill small buffer with white color.
    for (int i = 0; i < 240 * chunk_lines; i++)
    {
        // white hex num in 16 bit.
        color_buffer[i] = 0x0000;
    }
    // send the chunk to the screenmultiple times to cover whole screen.
    for (int j = 0; j < 320; j += chunk_lines)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, j, 240, j + chunk_lines, color_buffer);
    }

    free(color_buffer);
}