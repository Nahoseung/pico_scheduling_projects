#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H


#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#define NUM_OF_TASK 7
#define MAX_NUM_TASKS NUM_OF_TASK + configNUMBER_OF_CORES -1 
#define STACK_SIZE 256

#define Core0 (1<<0)
#define Core1 (1<<1)


typedef struct 
{
    TaskFunction_t Task_Code;
    char*  Task_Name;
    uint16_t Runtime;
    TickType_t Period;
    UBaseType_t Core_Affinity;
    uint8_t priority;
    uint8_t subnum;

    TaskHandle_t my_ptr;
    TaskHandle_t splitted_ptr;

    bool Dependency;
    bool Heavy;

    float Utilization;
}task_info;


typedef struct 
{
    task_info* list [MAX_NUM_TASKS];
    int top;
}task_stack;


typedef struct 
{
    UBaseType_t Core_num;
    float Utilization;
    bool pre_assigned;
}core_info;

typedef struct 
{
    core_info* list[configNUMBER_OF_CORES];
    int top;
    float Utilization_Bound;
}core_stack;







/************TASK**************/

float get_Utilization();
float get_lighttask(float U);

void init_task_stack(task_stack* task_stack_ptr);
float init_task(task_stack* task_stack_ptr, task_info task_list[],core_stack* core_stack_ptr);
bool Task_split(task_info* T, task_info* Tail_T,core_info* C, core_stack* core_stack_ptr,task_stack* task_stack_ptr);
void Assign_task(task_info* T,core_info* C);

bool T_is_full(task_stack* task_stack_ptr);
bool T_is_empty(task_stack* task_stack_ptr);
task_info* Pop_task(task_stack* task_stack_ptr);
void Push_task(task_info* T,task_stack* task_stack_ptr);
void Print_task(task_info* T);
bool simple_test(task_info* T, int num_of_Lower_T ,core_stack* core_stack_ptr,task_stack* task_stack_ptr);
task_info* new_task(int* idx,task_info task_list[]);

/************TASK**************/

/************Core**************/

void init_core_stack(core_stack* core_stack_ptr);
void init_core(core_stack* core_stack_ptr, core_info core_list[]);

bool C_is_full(core_stack* core_stack_ptr);
bool C_is_empty(core_stack* core_stack_ptr);
core_info* Pop_core(core_stack* core_stack_ptr);
void Push_core(core_info* P, core_stack* core_stack_ptr);

core_info* get_min_core(core_stack* core_stack_ptr);

/************Core**************/

int gcd(int a, int b);
int lcm(int a, int b);
int calculate_lcm(int periods[]);

bool Periodic_Job(task_info T, uint16_t Deadline);
#endif /*TASK_MANAGER_H*/