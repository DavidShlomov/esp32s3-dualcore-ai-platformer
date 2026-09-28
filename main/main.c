#include "lcd_setup.h"
#include "esp_random.h"
#include "esp_heap_caps.h"
#include <stdlib.h>
#include "main.h"


static int camera_x = 0;
static SemaphoreHandle_t dma_ready_sem = NULL;
static uint32_t random_num;

// Main entry point for the game engine
void app_main(void)
{
    uint16_t *front_buffer = allocate_screen_buffer();
    uint16_t *back_buffer = allocate_screen_buffer();
    uint16_t *temp_buffer;
    dma_ready_sem = xSemaphoreCreateBinary();
    esp_lcd_panel_handle_t panel_handle = lcd_setup_main(on_color_trans_done);
    int y_start_pos = 0;
    int character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    int map_index = 0;
    int map_offset = 0;
    int y_velocity = 0;
    
    uint8_t *map_level;
    Agent *agent_generation = NULL;
    Agent *champion_agent_array = NULL;//TODO: later add a champion array sized 10.

    xSemaphoreGive(dma_ready_sem);
    init_xorshift32();
    xorshift32();

    // Print initialization message
    printf("ESP32-S3 Game Engine Initialized!\n");

    map_level = build_map();

    agent_generation = allocate_agent_array(GENERATION_SIZE);
    init_agent_array(agent_generation, GENERATION_SIZE);

    simulate_agent_generation(agent_generation, map_level);

    qsort(agent_generation, GENERATION_SIZE, sizeof(Agent), compare_fitness);


    print_agent(&agent_generation[0]);
    print_map_array(map_level);

    natural_selection(agent_generation, map_level);
    print_agent(&agent_generation[0]);


    // Infinite loop to keep the task alive
    while (1)
    {
        //game tiles are obst width for simplicity.
        map_index = camera_x / OBSTACLE_WIDTH;
        map_offset = camera_x % OBSTACLE_WIDTH;
        camera_x += MOVE_RATE;

        // Calculate where the character is in the global map
        
        int character_global_x = camera_x + CIRCLE_CHARACTER_X_AXIS;
        int character_map_index = character_global_x / OBSTACLE_WIDTH;

        if(character_map_index >= DNA_SIZE){
            character_map_index = DNA_SIZE - 1;
        }

        xSemaphoreTake(dma_ready_sem, portMAX_DELAY);
        fill_screen_black(back_buffer);
        draw_road(back_buffer);

        draw_visible_obstacles(map_index, map_offset, y_start_pos, map_level, back_buffer);

        agent_jump(&agent_generation[0], &y_velocity, character_map_index, &character_y_axis);

        draw_circle(back_buffer, CIRCLE_CHARACTER_X_AXIS, character_y_axis, CIRCLE_CHARACTER_RADIUS);

        check_collision(character_global_x,character_y_axis, map_level, &agent_generation[0].is_dead);

        temp_buffer = front_buffer;
        front_buffer = back_buffer;
        back_buffer = temp_buffer;

        flush_screen_buffer(panel_handle, front_buffer);

        check_game_over(map_index, &character_y_axis, &agent_generation[0].is_dead, &y_velocity , panel_handle);

    }
}

void draw_road(uint16_t *screen_buffer)
{
    for (int i = 0; i < ROAD_SIZE; i++)
    {
        screen_buffer[i] = PURPLE_COLOR;
    }
}

void create_obstacle(uint16_t *screen_buffer, int height, int width, int x_axis_start, int y_axis_start)
{

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int current_x = x_axis_start + x;
            int current_y = ROAD_HEIGHT + y_axis_start + y;

            int current_index = (current_y * SCREEN_WIDTH) + current_x;

            if (current_x < 0 || current_x >= SCREEN_WIDTH)
            {
                continue;
            }

            screen_buffer[current_index] = WHITE_COLOR;
        }
    }
}

// circle character for simplicity.
void draw_circle(uint16_t *screen_buffer, int x_center, int y_center, int radius)
{
    for (int y = 0; y < 2 * radius; y++)
    {
        for (int x = 0; x < 2 * radius; x++)
        {
            int current_x = x_center - radius + x;
            int current_y = y_center - radius + y;
            int current_index = (current_y * SCREEN_WIDTH) + current_x;

            if (radius * radius >= (x - radius) * (x - radius) + (y - radius) * (y - radius))
            {
                screen_buffer[current_index] = BURGUNDY_COLOR;
            }
        }
    }
}

//randomizer
void xorshift32()
{
    random_num ^= random_num << 13;
    random_num ^= random_num >> 17;
    random_num ^= random_num << 5;
}

void init_xorshift32()
{
    random_num = esp_random();

    if (random_num == 0)
    {
        random_num = 1;
    }
}

//init psram buffer.
uint16_t *allocate_screen_buffer()
{
    uint16_t *screen_buffer = (uint16_t *)heap_caps_malloc(SCREEN_SIZE * sizeof(uint16_t), MALLOC_CAP_SPIRAM);

    if (!screen_buffer)
    {
        printf("Error: screen buffer not initialized!");
        while (1)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    return screen_buffer;
}

void fill_screen_black(uint16_t *screen_buffer)
{

    // memset(screen_buffer, INVERTED_WHITE, screen_size * sizeof(uint16_t));
    for (int i = 0; i < SCREEN_SIZE; i++)
    {
        screen_buffer[i] = BLACK_COLOR;
    }
}

void flush_screen_buffer(esp_lcd_panel_handle_t panel_handle, uint16_t *screen_buffer)
{
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 320, 240, screen_buffer);
}

// ISR for SPI DMA transfer complete.
IRAM_ATTR bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(dma_ready_sem, &high_task_awoken);

    return (high_task_awoken == pdTRUE);
}

//generate random map array.
uint8_t *build_map()
{
    uint8_t *map_array = calloc(MAP_SIZE, sizeof(uint8_t));
    //starting with 12 empty cells, at least 2 last tiles empty.
    for (int i = NUM_OF_TILES_IN_SCREEN; i < MAP_SIZE - 2; i++)
    {
        if (random_num % 2 == 0)
        {
            map_array[i] = random_num % 90 + 40;
            i += 3;
        }
        xorshift32();
    }

    return map_array;
}

void print_map_array(const uint8_t *map_array)
{
    for (int i = 0; i < MAP_SIZE; i++)
    {
        printf("%d ", map_array[i]);
    }
    printf("\n");
}

//draw LEVEL END on screen.
void show_level_end_screen(esp_lcd_panel_handle_t panel_handle)
{
    // Initialize buffer and paint it black
    uint16_t *end_screen = allocate_screen_buffer();
    fill_screen_black(end_screen);

    // Bitmap matrix for "LEVEL END" (5 rows, 37 columns)
    // 1 = White pixel, 0 = Black background
    const uint8_t text_bitmap[5][37] = {
        {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0},
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0},
        {1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0}};

    // Configuration for retro scaling and screen centering
    int scale = 4; // Each '1' in the bitmap becomes a 4x4 pixel square
    int bitmap_width = 37;
    int bitmap_height = 5;

    // Calculate total size of the text on the screen
    int total_width = bitmap_width * scale;
    int total_height = bitmap_height * scale;

    // Calculate starting X and Y to perfectly center the text
    int start_x = (SCREEN_WIDTH - total_width) / 2;
    int start_y = (SCREEN_HEIGHT - total_height) / 2;

    // Render the bitmap into the screen buffer
    for (int row = 0; row < bitmap_height; row++)
    {
        for (int col = 0; col < bitmap_width; col++)
        {
            if (text_bitmap[row][col] == 1)
            {
                // Draw a scaled block for each active pixel in the bitmap
                for (int dy = 0; dy < scale; dy++)
                {
                    for (int dx = 0; dx < scale; dx++)
                    {
                        int screen_x = start_x + (col * scale) + dx;
                        int screen_y = start_y + ((bitmap_height - 1 - row) * scale) + dy;

                        // Prevent drawing out of bounds just in case
                        if (screen_x >= 0 && screen_x < SCREEN_WIDTH && screen_y >= 0 && screen_y < SCREEN_HEIGHT)
                        {
                            int index = (screen_y * SCREEN_WIDTH) + screen_x;
                            end_screen[index] = WHITE_COLOR;
                        }
                    }
                }
            }
        }
    }

    // Send the buffer to the physical display
    flush_screen_buffer(panel_handle, end_screen);

    // Pause the game for 3 seconds to let the player read it
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Free the dynamically allocated memory to prevent memory leaks
    free(end_screen);
}

void draw_visible_obstacles(int map_index, int map_offset, int y_start_pos ,uint8_t *curr_level, uint16_t *screen_buffer){

            //drawing map based on map array heights.
        for (int i = map_index; i < map_index + NUM_OF_TILES_IN_SCREEN; i++)
        {
            if (i == MAP_SIZE)
            {
                break;
            }
            if (curr_level[i] != 0)
            {
                create_obstacle(screen_buffer, curr_level[i], OBSTACLE_WIDTH, ((i - map_index) * OBSTACLE_WIDTH) - map_offset, y_start_pos);
            }
        }
}

void check_collision(int character_global_x, int character_y_axis, uint8_t *curr_level, bool *is_dead){
    // Calculate the global X of the character's right and left edges
    int char_right_edge = character_global_x + CIRCLE_CHARACTER_RADIUS;
    int char_left_edge = character_global_x - CIRCLE_CHARACTER_RADIUS;
    int char_bottom_edge = character_y_axis - CIRCLE_CHARACTER_RADIUS;

    // Find the range of tiles the character is currently overlapping (Left and Right)
    int start_index = char_left_edge / OBSTACLE_WIDTH;
    int end_index = char_right_edge / OBSTACLE_WIDTH;

    // Loop through all tiles the character touches to prevent clipping through corners
    for (int idx = start_index; idx <= end_index; idx++)
    {
        // Check if there is an actual obstacle in this specific index
        if (idx >= 0 && idx < MAP_SIZE && curr_level[idx] != 0)
        {
            // Extract the obstacle's parameters
            int obs_left_edge = idx * OBSTACLE_WIDTH;
            int obs_right_edge = obs_left_edge + OBSTACLE_WIDTH;
            int obs_top_edge = ROAD_HEIGHT + curr_level[idx];

            // Mathematical overlap check
            if (char_right_edge > obs_left_edge && char_left_edge < obs_right_edge && char_bottom_edge < obs_top_edge)
            {
                *is_dead = true;
                break; // Once we hit something, no need to check other tiles
            }
        }
    }
}


void check_game_over(int map_index, int *character_y_axis, bool *is_dead, int *y_velocity , esp_lcd_panel_handle_t panel_handle){

    //maybe later add different screens for level finish and death.
    if (map_index >= MAP_SIZE || *is_dead)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS); // Give a half second to see the death
        show_level_end_screen(panel_handle);
        camera_x = 0;
        *character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
        *is_dead = false;
        *y_velocity = 0;
    }
}

//Agent jumping logic need to callibrate numbers later.
void agent_jump(Agent *agent, int *y_velocity, int dna_array_index, int *agent_y_axis){
    int gravity = 1;

    if(dna_array_index >= DNA_SIZE){
        printf("dna_array_index is out of bound.\n");
        return;
    }
    //Agent is currenly not mid jump. 
    if(*y_velocity == 0 && *agent_y_axis == CIRCLE_CHARACTER_INITIAL_Y){
        switch (agent->dna[dna_array_index])
        {
            case LOW:
                *y_velocity = 16;
                break;
            case MEDIUM:
                *y_velocity = 18;
                break;
            case HIGH:
                *y_velocity = 20;
                break;
            default:
                break;
        }
    }
    //Physics
    *y_velocity -= gravity;
    *agent_y_axis += *y_velocity;
    //Physics corretion
    if(*agent_y_axis <= CIRCLE_CHARACTER_INITIAL_Y){
        *y_velocity = 0;
        *agent_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    }
}

//Headless simulator for agent training without rendering to screen.
//Supposed to run a single agent until it dies or passes. 
void simulate_agent(Agent *agent, uint8_t *curr_level){
    int character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    int y_velocity = 0;
    int character_x = 0;
    int character_map_index = 0;
    bool above_obstacle = false;

    xorshift32();

    while (!(agent->is_dead) && character_map_index < MAP_SIZE)
    {
        //Game tiles are obstacle width.
        character_map_index = character_x / OBSTACLE_WIDTH;

        if(character_map_index >= MAP_SIZE){
            break;
        }
        
        calculate_fitness(&agent->fitness, character_y_axis, character_map_index, &above_obstacle, curr_level);

        agent_jump(agent, &y_velocity, character_map_index, &character_y_axis);

        check_collision(character_x,character_y_axis, curr_level, &agent->is_dead);
        
        character_x += MOVE_RATE;
    }

    agent->distance = character_map_index;
}

void calculate_fitness(int *fitness, int agent_y_axis, int map_index, bool *above_obstacle, uint8_t *curr_level){

    //agent is above an obstacle.
    if(agent_y_axis > CIRCLE_CHARACTER_INITIAL_Y && curr_level[map_index] > 0){
        *above_obstacle = true;
    }
    //agent passed obstacle successfully.
    if(*above_obstacle && agent_y_axis == CIRCLE_CHARACTER_INITIAL_Y){
        *fitness += 100;
        *above_obstacle = false;
    }
    //airtime fitmess is -1.
    if(agent_y_axis > CIRCLE_CHARACTER_INITIAL_Y){
        *fitness -= 1;
    }
    //last tile guranteed with no obstacle, map is passed.
    if(map_index == MAP_SIZE - 1){
        *fitness += 1000;
    }
}

//initiate an agent with random dna.
void init_agent(Agent *new_agent){
    new_agent->distance = 0;
    new_agent->fitness = 0;
    new_agent->is_dead = false;
    randomize_agent_dna(new_agent);
}

//4 options: 0 = no jump, 1 = low, 2 = medium, 3 = high.
void randomize_agent_dna(Agent *new_agent){

    for(int i = 0; i < DNA_SIZE; i++){
        new_agent->dna[i] = random_num % 4;
        xorshift32();
    }
}

void print_agent(Agent *agent){

    printf("Agent fitness is: %d\n", agent->fitness);
    printf("Agent death status is: %s\n", agent->is_dead ? "true" : "false");
    printf("Agent distance is: %d\n", agent->distance);
    printf("Agent dna is: \n");
    print_map_array(agent->dna);
}

Agent *allocate_agent_array(int array_size){
    Agent *gen_zero = (Agent *) malloc(sizeof(Agent) * array_size);

    if(gen_zero == NULL){
        printf("Agent generation memory allocation failed.\n");
    }
    return gen_zero;
}

void init_agent_array(Agent *agent_generation, int array_size){

    for(int i = 0; i < array_size; i++){
        init_agent(&agent_generation[i]);
    }
}

void print_agent_array(Agent *agent_generation, int array_size){
    
    for(int i = 0; i < array_size; i++){
        printf("Agent %d:\n", i + 1);
        print_agent(&agent_generation[i]);
    }
}

Agent *allocate_champion_array(int array_size){
    Agent *champion_array = (Agent *)calloc(array_size, sizeof(Agent));

    if(champion_array == NULL){
        printf("Could not allocate champion array");
    }
    return champion_array;
}

void simulate_agent_generation(Agent *agent_array, uint8_t *current_level){
    for(int i = 0; i < GENERATION_SIZE; i++){
        simulate_agent(&agent_array[i], current_level);
    }
}

//Comparioson function for qsort.
int compare_fitness(const void *agent_a, const void *agent_b){
    return ((Agent *)agent_b)->fitness - ((Agent *)agent_a) -> fitness;
}

void natural_selection(Agent *agent_generation, uint8_t *map_level){
    int generation_num = 0;
    int best_distance = 0;
    int local_optimum_check = 0;
    int mutation_rate_percent = 1;
    uint8_t *parent_a;
    uint8_t *parent_b;
    uint8_t *child_to_evolve;

    //random number of generations will change later.
    while(generation_num < NUM_OF_GENERATIONS){


        if(generation_num % 100 == 0){
            printf("Gen %d: best fitness = %d, best distance = %d\n", generation_num, agent_generation[0].fitness, agent_generation[0].distance);
            //best_distance is not at local-optimum.
            if(best_distance != agent_generation[0].distance){
                best_distance = agent_generation[0].distance;
                local_optimum_check = 0;
                mutation_rate_percent = 1;   
            }//best_distance is at local-optimum.
            else{
                local_optimum_check++;
                //local_optimum is consistent - need to shake up with mutation percent increase. 
                if(local_optimum_check >= 3){
                    mutation_rate_percent += 2;
                    if(mutation_rate_percent > 10){
                        mutation_rate_percent = 10;
                    }
                    local_optimum_check = 0;
                }
            }
        }

        for(int i = GENERATION_SIZE/2; i < GENERATION_SIZE; i++){
            xorshift32();
            parent_a = agent_generation[random_num % (GENERATION_SIZE/2)].dna;
            xorshift32();
            parent_b = agent_generation[random_num % (GENERATION_SIZE/2)].dna;
            child_to_evolve = agent_generation[i].dna;

            double_cross_over(parent_a, parent_b, child_to_evolve);
            mutation(child_to_evolve, mutation_rate_percent);
        }

        generation_reset(agent_generation);
        simulate_agent_generation(agent_generation, map_level);
        generation_num++;
        vTaskDelay(1);//breathing room for watchdog timeout.
        qsort(agent_generation, GENERATION_SIZE, sizeof(Agent), compare_fitness);
    }

    print_agent(&agent_generation[0]);
}

void double_cross_over(uint8_t *parent_a_dna, uint8_t *parent_b_dna, uint8_t *child_dna){
    int first_cross_point;
    int second_cross_point;
    int temp_cross_point;
    int random_main_parent;
    
    xorshift32();
    random_main_parent = random_num % 2;
    xorshift32();
    first_cross_point = random_num % DNA_SIZE;
    xorshift32();
    second_cross_point = random_num % DNA_SIZE;

    //assuring first_cross_point < second_cross_point, for loop efficiency.
    if(first_cross_point > second_cross_point){
        temp_cross_point = first_cross_point;
        first_cross_point = second_cross_point;
        second_cross_point = temp_cross_point; 
    }

    //parent_a is main genetics.
    if(random_main_parent == 0){
            
        for(int i = 0; i < first_cross_point; i++){
            child_dna[i] = parent_a_dna[i];
        }

        for(int i = first_cross_point; i < second_cross_point; i++){
            child_dna[i] = parent_b_dna[i];
        }

        for(int i = second_cross_point; i < DNA_SIZE; i++){
            child_dna[i] = parent_a_dna[i];
        }
    }//parent_b is main genetics
    else{
        for(int i = 0; i < first_cross_point; i++){
            child_dna[i] = parent_b_dna[i];
        }

        for(int i = first_cross_point; i < second_cross_point; i++){
            child_dna[i] = parent_a_dna[i];
        }

        for(int i = second_cross_point; i < DNA_SIZE; i++){
            child_dna[i] = parent_b_dna[i];
        }
    }
}

void mutation(uint8_t *child_dna, float mutation_rate_percent){
    
    for(int i = 0; i < DNA_SIZE; i ++){
        xorshift32();

        if((random_num % DNA_SIZE) < mutation_rate_percent){
            //definit change in the cell if mutation occurs.
            child_dna[i] += ((random_num % 3) + 1);
            child_dna[i] = child_dna[i] % 4;
        }
    }
}

void generation_reset(Agent *agent_generation){

    for(int i = 0; i < GENERATION_SIZE; i++){
        agent_generation[i].is_dead = false;
        agent_generation[i].fitness = 0;
        agent_generation[i].distance = 0;
    }
}

/*
            ****TO DO****
            Training time is super long (7:14 min for 10,000 gens, 4:07 for 5000 gens) need optimization, 10000 generations basically "guranteed" level passed.
            dna array out of bond at the end of the map.

            
            ****SCALING****
            make consectual agents running
            mid run new map agent training, running in succesion.
            split into 2 cores.


            ****BONUS****
            calculate actual jump peak of velocity to make the highest option go exactly over the highest obctacle.
*/