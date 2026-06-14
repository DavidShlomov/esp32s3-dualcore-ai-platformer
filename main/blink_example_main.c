#include "lcd_setup.h"

// Main entry point for the game engine
void app_main(void)
{
    esp_lcd_panel_handle_t my_screen = lcd_setup_main();
    // Print initialization message
    printf("ESP32-S3 Game Engine Initialized!\n");

    // Infinite loop to keep the task alive
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
