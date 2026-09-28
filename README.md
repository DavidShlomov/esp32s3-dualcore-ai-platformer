# GameEngine - Bare-Metal Genetic Algorithm Platformer (ESP32-S3)

A 2D obstacle-jumping platformer built from scratch in C on the ESP32-S3, with a custom SPI/DMA display driver and a from-scratch genetic algorithm that trains an AI agent, headlessly, to clear a procedurally generated level - then plays back the evolved result live on a physical LCD.

No Arduino libraries, no ML frameworks, no OS-level graphics stack - just ESP-IDF, FreeRTOS, and raw C.

## What it does

1. **Generates a random obstacle course** - a sequence of variable-height obstacles with guaranteed minimum spacing, so every run trains against a different level.
2. **Evolves a population of 50 agents** against that level, headlessly (no rendering, as fast as the CPU allows): each agent's "DNA" is a sequence of jump decisions (one per tile - do nothing / jump low / medium / high), evaluated frame-by-frame against the same physics and collision logic used by the live game.
3. **Selects, crosses over, and mutates** the population generation over generation, with an adaptive mutation rate that increases automatically when the population's progress plateaus.
4. **Renders the best-evolved agent live** on a physical ILI9341 TFT over SPI/DMA, double-buffered, with the display update fully decoupled from the render loop via an ISR-driven semaphore handoff.

## Why this isn't just "a platformer with an AI bolted on"

- **The display driver is hand-written**, not a library call. SPI bus init, panel bring-up, and double-buffered rendering are synchronized correctly with the DMA hardware via a binary semaphore released from the SPI completion ISR - so the CPU never writes into a buffer the display controller is still reading from mid-transfer.
- **The physics and the training share one code path.** The same jump/gravity/collision functions run whether a human would be watching (live render loop) or an agent is being evaluated in the training loop - so the genetic algorithm is training against the actual game, not an approximation of it.
- **Height clearance is a real constraint, not just tuning.** Each obstacle has to be cleared not just at its peak height but for the entire width of the jump arc that overlaps it - the maximum obstacle height any jump strength can clear is a closed-form function of its initial velocity, not something guessed by trial and error.
- **Hit and diagnosed a real FreeRTOS watchdog trip.** Running thousands of generations synchronously starved the idle task long enough to trigger the task watchdog - fixed by yielding periodically inside the training loop, not by disabling the watchdog.
- **Runs inside real embedded memory constraints.** PSRAM is used explicitly for the double frame buffers (`heap_caps_malloc` with `MALLOC_CAP_SPIRAM`); the RNG is a self-seeded xorshift32 stream rather than a library call, reseeded once from the hardware TRNG at boot.

## Architecture

- `lcd_setup.c/.h` - SPI bus and ILI9341 panel initialization, GPIO pin configuration, DMA-driven full-screen writes.
- `main.c` - game loop, physics, collision detection, procedural map generation, and the full genetic algorithm (population init, headless simulation, selection, crossover, mutation, adaptive mutation rate, fitness scoring).
- `main.h` - shared constants, the `Agent` struct (DNA array + fitness/distance/death state), and Doxygen-documented function declarations.

## Hardware

- ESP32-S3 (Xtensa dual-core, though the current build runs single-core - see Roadmap)
- 2.8" ILI9341 TFT LCD, SPI interface
- ESP-IDF v6.0.1 / FreeRTOS

## Building and running

```
idf.py build flash monitor
```
Watch the serial monitor for per-generation fitness/distance progress during training, then the display will show the evolved agent playing the level live.

## Roadmap

- [ ] Move training onto the second core, running concurrently with live rendering (currently all training happens synchronously before the render loop starts).
- [ ] Persistent "champion" storage - a fixed-size ring buffer of best-performing agents, so a new map can start training while a previous champion's run is still being displayed.
- [ ] Random-immigrant injection and other diversity mechanisms to reduce premature convergence on harder maps.

## What I'd do differently

The genetic algorithm's parameters (population size, mutation rate, generation cap) were tuned empirically against runtime constraints on real hardware, not derived analytically - a natural next step would be logging convergence data across multiple seeded runs to tune these more rigorously.
