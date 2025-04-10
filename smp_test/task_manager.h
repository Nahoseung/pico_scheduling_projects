#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#define NUM_OF_TASKS 5
#define false 0
#define true 1

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"


typedef struct 
{
    uint16_t Runtime;
    TickType_t Period;
}task_manager;

// typedef enum 
// {
//     false,
//     true
// }bool;


void Periodic_Job(uint16_t Runtime,uint16_t Deadline);
void Task_split();
void SPA2();




#endif /*TASK_MANAGER_H*/