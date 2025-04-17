#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H


#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#define NUM_OF_TASK 3
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
}core_info;

typedef struct 
{
    core_info* list[configNUMBER_OF_CORES];
    int top;
}core_stack;





bool Periodic_Job(uint16_t Runtime, uint16_t Deadline);
void init_task(task_stack* admin, task_info task_list[],core_stack* core_stack_ptr);
void init_admin(task_stack* admin, task_info* manager);
bool Task_split(uint8_t Body_idx, task_stack* admin, core_info* p_manager[]);
void Assign_task(TaskHandle_t task_ptr,UBaseType_t Core_num, float Utilization, core_stack* core_stack_ptr);
/************NOT YET**************/

void SPA2();

void init_task_stack(task_stack* task_stack_ptr);
bool T_is_full(task_stack* task_stack_ptr);
bool T_is_empty(task_stack* task_stack_ptr);
task_info* Pop_task(task_stack* task_stack_ptr);
void Push_task(task_info* T,task_stack* task_stack_ptr);
void Print_task(task_info* T);

void init_core_stack(core_stack* core_stack_ptr);
bool C_is_full(core_stack* core_stack_ptr);
bool C_is_empty(core_stack* core_stack_ptr);
void Push_core(core_info* P, core_stack* core_stack_ptr);
core_info* Pop_core(core_stack* core_stack_ptr);
void init_core(core_stack* core_stack_ptr, core_info core_list[]);

float get_Utilization();
float get_lighttask(float U);


#endif /*TASK_MANAGER_H*/