#include "task_manager.h"


void Periodic_Job(uint16_t Runtime,uint16_t Deadline)
{
    /********* CHECK DEADLINE ********/
    if(xTaskGetTickCount() + Runtime >= Deadline)
    {
        printf("OVERFLOW \n ");
        sleep_ms(5000);
        return ;
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
        return ;
    }
    return ;
}

