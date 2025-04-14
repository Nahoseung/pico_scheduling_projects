#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H


#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#define NUM_OF_TASK 2
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
}task_info;

typedef struct 
{
    task_info* manager;
    uint8_t top;
}administrator;


void init_task(administrator* admin);
bool Periodic_Job(uint16_t Runtime, uint16_t Deadline);
void init_admin(administrator* admin, task_info* manager);
bool Task_split(uint8_t Body_idx, administrator* admin);

/************NOT YET**************/
void SPA2();


#endif /*TASK_MANAGER_H*/