#include "task_manager.h"



bool Periodic_Job(uint16_t Runtime,uint16_t Deadline)
{
    /********* CHECK DEADLINE ********/
    if(xTaskGetTickCount() + Runtime >= Deadline)
    {
        printf("OVERFLOW \n ");
        sleep_ms(5000);
        return false;
    }

    /********* RUN TASK ********/
    for(int i=0;i<Runtime;i++)
    {
        sleep_ms(10); // 1 Tick
    }

    /********* CHECK DEADLINE ********/
    if(xTaskGetTickCount()> Deadline)
    {
        printf("OVERFLOW : MISS DEADLINE\n");
        sleep_ms(5000);
        return false;
    }
}


void init(task_manager manager[])
{
    for(int i=0; i<NUM_OF_TASK; i++)
    {
        xTaskCreateAffinitySet(manager[i].Task_Code,manager[i].Task_Name,STACK_SIZE,NULL,i+1,manager[i].Core_Affinity,NULL);
    }
}

