#include "genetic_algorithm.h"
#include "prng.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"







//Headless simulator for agent training without rendering to screen.
//Supposed to run a single agent until it dies or passes. 
void simulate_agent(Agent *agent, uint8_t *curr_level){
    int character_y_axis = CIRCLE_CHARACTER_INITIAL_Y;
    int y_velocity = 0;
    int character_x = 0;
    int character_map_index = 0;
    bool above_obstacle = false;
    bool received_map_pass_points = false;


    while (!(agent->is_dead) && character_map_index < MAP_SIZE)
    {
        //Game tiles are obstacle width.
        character_map_index = character_x / OBSTACLE_WIDTH;

        if(character_map_index >= MAP_SIZE){
            break;
        }
        
        calculate_fitness(&agent->fitness, character_y_axis, character_map_index, &above_obstacle, curr_level, &received_map_pass_points);

        agent_jump(agent, &y_velocity, character_map_index, &character_y_axis);

        check_collision(character_x,character_y_axis, curr_level, &agent->is_dead);
        
        character_x += MOVE_RATE;
    }

    agent->distance = character_map_index;
}


void simulate_agent_generation(Agent *agent_array, uint8_t *current_level, int simulation_index_start){
    for(int i = simulation_index_start; i < GENERATION_SIZE; i++){
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
    int elite_cutoff = GENERATION_SIZE/2;
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
        //only changes dna after elite_cutoff index.
        for(int i = elite_cutoff; i < GENERATION_SIZE; i++){
            parent_a = agent_generation[get_random_num() % (elite_cutoff)].dna;
            parent_b = agent_generation[get_random_num() % (elite_cutoff)].dna;
            child_to_evolve = agent_generation[i].dna;

            double_cross_over(parent_a, parent_b, child_to_evolve);
            mutation(child_to_evolve, mutation_rate_percent);
        }
        //only resets after elite_cutoff index - safe, only later half of the array is rewritten.
        generation_reset(agent_generation, elite_cutoff);
        simulate_agent_generation(agent_generation, map_level, elite_cutoff);//simulating only agent indexes that changed dna.
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
    
    random_main_parent = get_random_num() % 2;
    first_cross_point = get_random_num() % DNA_SIZE;
    second_cross_point = get_random_num() % DNA_SIZE;

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

        if((get_random_num() % DNA_SIZE) < mutation_rate_percent){
            //definit change in the cell if mutation occurs.
            child_dna[i] += ((get_random_num() % 3) + 1);
            child_dna[i] = child_dna[i] % 4;
        }
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
        new_agent->dna[i] = get_random_num() % 4;
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

void generation_reset(Agent *agent_generation, int reset_index_start){

    for(int i = reset_index_start; i < GENERATION_SIZE; i++){
        agent_generation[i].is_dead = false;
        agent_generation[i].fitness = 0;
        agent_generation[i].distance = 0;
    }
}
