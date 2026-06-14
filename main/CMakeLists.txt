idf_component_register(SRCS "main.c" "lcd_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_lcd espressif__esp_lcd_ili9341)
