idf_component_register(SRCS "render.c" "genetic_algorithm.c" "game.c" "prng.c" "main.c" "lcd_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_lcd espressif__esp_lcd_ili9341)
