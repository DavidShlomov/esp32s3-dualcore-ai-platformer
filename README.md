# GameEngine: Genetic Algorithm Platformer on ESP32-S3

A 2D obstacle-jumping game written in C for the ESP32-S3, with a hand-written SPI/DMA display driver and a genetic algorithm that trains an agent to clear a randomly generated level. Training runs headlessly on the chip itself, then the evolved agent plays the level live on a physical LCD.

No Arduino libraries and no ML frameworks. Just ESP-IDF, FreeRTOS, and C.

## What it does

1. **Generates a random obstacle course.** Obstacle heights are random, with guaranteed empty tiles at the start and end and a minimum gap between obstacles, so every boot produces a new level.
2. **Evolves a population of 50 agents.** Each agent's DNA is one jump decision per map tile (none, low, medium, high). Agents are simulated without rendering, using the same physics and collision code as the live game.
3. **Selects, crosses over, and mutates.** The top half of the population is kept each generation. The bottom half is rebuilt with two-point crossover and mutation. The mutation rate rises automatically when progress stalls.
4. **Plays back the best agent live** on an ILI9341 TFT over SPI with DMA and double-buffered frames.

## Technical highlights

- **Hand-written display driver.** SPI bus setup, panel bring-up, and frame transfer are done directly against ESP-IDF's LCD APIs. A binary semaphore, given from the DMA completion ISR, makes sure the CPU never writes into a buffer the display is still reading.
- **No shared globals between modules.** The ISR receives the semaphore through the driver's `user_ctx` pointer, and loop state such as `camera_x` is passed by pointer. Each module only touches what it is handed.
- **One code path for training and playback.** Jump physics, collision, and fitness live in `game.c` and are used by both the headless simulator and the live loop, so the agent trains on the real game rather than an approximation.
- **Fitness shaped by the physics.** An agent earns +100 for clearing an obstacle, loses 1 point per tick spent in the air (which favors the lowest jump that works), and gets a one-time +1000 for finishing the map. The three jump strengths were chosen from the jump-arc math: a jump has to stay above an obstacle for its whole width, not just reach its height once.
- **Training cost cut in half.** Elite agents keep their DNA between generations, and simulation is deterministic, so only the rebuilt half of the population is re-simulated each generation.
- **Real embedded constraints.** Frame buffers live in PSRAM (`heap_caps_malloc` with `MALLOC_CAP_SPIRAM`). The RNG is a small xorshift32 generator that seeds itself from the hardware RNG on first use.
- **Debugged on hardware.** Running thousands of generations in one task tripped the FreeRTOS task watchdog. The fix was a short yield each generation, not disabling the watchdog.

## Project layout

| File | Responsibility |
|---|---|
| `main.c` | `app_main`: setup, training, then the live render loop |
| `configs.h` | Shared constants (screen, colors, physics, population sizes) and the jump enum |
| `game.c/.h` | Map generation, collision, jump physics, fitness (safe to call without a display) |
| `genetic_algorithm.c/.h` | `Agent` type, population handling, selection, crossover, mutation |
| `render.c/.h` | Drawing, frame buffers, the DMA completion callback, game-over screen |
| `lcd_setup.c/.h` | SPI bus, LCD IO layer, and ILI9341 panel initialization |
| `prng.c/.h` | xorshift32 random number generator behind a single `get_random_num()` |

## Hardware

- ESP32-S3 with PSRAM
- 2.8" ILI9341 TFT, 320x240, SPI
- ESP-IDF v6.0.1

| LCD signal | GPIO |
|---|---|
| MOSI | 15 |
| SCLK | 16 |
| CS | 5 |
| DC | 7 |
| RST | 6 |

SPI clock is 40 MHz.

## Build and run

```
idf.py build flash monitor
```

The serial monitor prints best fitness and distance every 100 generations during training. When training finishes, the display shows the best agent running the level.

## Roadmap

- [ ] Run training on the second core while the first core renders, so a new map can train during playback (training is currently about 50 seconds and blocks before playback starts).
- [ ] Champion storage: a fixed-size ring buffer of the best agents, each paired with the map it solved.
- [ ] Diversity mechanisms such as random immigrants, to reduce premature convergence on harder maps.
- [ ] Bounds checking in obstacle drawing for the vertical axis.

## What I would do differently

The population size, mutation rates, and generation cap were tuned by hand against training time on the real hardware. A better approach is to log convergence over many seeded runs and tune from that data. I would also write a small deterministic test (fixed seed, fixed expected fitness) before refactoring, so a change in behavior shows up immediately instead of being checked by eye.
