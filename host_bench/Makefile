bench: bench_main.c stubs/esp_stubs.c stubs/esp_heap_caps.h stubs/esp_log.h stubs/esp_random.h \
		stubs/freertos/FreeRTOS.h stubs/freertos/semphr.h stubs/freertos/task.h \
		../main/game.c ../main/game.h ../main/genetic_algorithm.c ../main/genetic_algorithm.h \
		../main/prng.c ../main/prng.h ../main/configs.h
	gcc -Wall -Wextra -Werror -I stubs -I ../main bench_main.c stubs/esp_stubs.c ../main/prng.c ../main/game.c ../main/genetic_algorithm.c -o bench
