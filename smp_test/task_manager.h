#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#define NUM_OF_TASKS 5

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
    const char* const Task_Name;
    uint16_t Runtime;
    TickType_t Period;
    UBaseType_t Core_Affinity;
/************NOT YET**************/
    TaskHandle_t my_ptr;
    TaskHandle_t splitted_ptr;
    bool Dependency;
}task_manager;


void init(task_manager* manager);
bool Periodic_Job(uint16_t Runtime,uint16_t Deadline);

/************NOT YET**************/
void SPA2();
void Task_split();

#endif /*TASK_MANAGER_H*/