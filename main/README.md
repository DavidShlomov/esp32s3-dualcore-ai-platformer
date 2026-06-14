# ESP32-S3 Dual-Core AI Platformer

A bare-metal hardware project on the ESP32-S3 microcontroller, featuring a custom hardware-accelerated 2D platformer engine on one core, and a real-time procedural map generator and genetic learning algorithm on the second core.

## System Architecture

This project leverages the dual-core Xtensa LX7 processor, utilizing FreeRTOS tasks to pin heavy computational loads to specific cores:

- **Core 0 (AI & Generation):** Dedicated to procedural map randomization and running a genetic learning algorithm (handling population evaluation, crossover, and mutation) to solve the generated obstacles.
- **Core 1 (Game Engine & Rendering):** Manages the physical rendering pipeline via SPI and DMA, driving an ILI9341 TFT display, updating entity components, and preventing heap overflow via chunked buffer drawing.

## Hardware Stack

- **Microcontroller:** ESP32-S3
- **Display:** 2.8" ILI9341 TFT LCD (SPI)
- **Framework:** ESP-IDF (C)

## Development Roadmap

### Phase 1: Hardware Abstraction & Graphics (Core 1)

- [x] SPI bus initialization and hardware IO configuration.
- [x] LCD Driver integration and Opaque pointer (handle) management.
- [x] Memory chunking implementation for clean rendering without buffer overflows.
- [ ] 2D platformer entity mechanics (physics, jumping, simple obstacles).

### Phase 2: Artificial Intelligence (Core 0)

- [ ] Procedural map and obstacle generation algorithm.
- [ ] Genetic learning logic framework.
- [ ] Fitness function design for obstacle avoidance.

### Phase 3: System Integration

- [ ] Inter-Process Communication (IPC) utilizing FreeRTOS queues to sync Map data between Core 0 and Core 1.
- [ ] Real-time visualization of the AI learning process on the physical display.
