idf_component_register(SRCS "blink_example_main.c" "lcd_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_lcd espressif__esp_lcd_ili9341)
