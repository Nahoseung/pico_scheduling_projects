#include "task_manager.h"

bool Periodic_Job(uint16_t Runtime, uint16_t Deadline)
{
    /********* CHECK DEADLINE ********/
    if(xTaskGetTickCount() + Runtime >= Deadline)
    {
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
        return false;
    }
    return true;
}


void init_task(administrator* admin)
{
    for(int i=0; i < NUM_OF_TASK; i++)
    {
        xTaskCreateAffinitySet(admin->manager[i].Task_Code,admin->manager[i].Task_Name,STACK_SIZE,NULL,admin->manager[i].priority,admin->manager[i].Core_Affinity,&(admin->manager[i].my_ptr));
    }
}


void init_admin(administrator* admin, task_info* manager)
{
    admin->manager = manager;
    admin->top = NUM_OF_TASK - 1;
}

bool Task_split(uint8_t Body_idx, administrator* admin)
{

    if( ++ admin-> top >= MAX_NUM_TASKS )
    {
        printf("NO MORE SPLIT \n");
        return false;
    }
    uint8_t Tail_idx = admin->top;

    admin->manager[Tail_idx].Task_Name = admin->manager[Body_idx].Task_Name;
    admin->manager[Tail_idx].Runtime = (admin->manager[Body_idx].Runtime) / 2;

    admin->manager[Tail_idx].Period = admin->manager[Body_idx].Period;
    admin->manager[Tail_idx].Core_Affinity = Core1;

    admin->manager[Tail_idx].priority = admin->manager[Body_idx].priority;
    admin->manager[Tail_idx].subnum = (admin->manager[Body_idx].subnum + 1);
    admin->manager[Tail_idx].Dependency = true;
    
    xTaskCreateAffinitySet(admin->manager[Tail_idx].Task_Code,admin->manager[Tail_idx].Task_Name,STACK_SIZE,NULL,admin->manager[Tail_idx].priority,admin->manager[Tail_idx].Core_Affinity,&(admin->manager[Tail_idx].my_ptr));

    admin->manager[Body_idx].Runtime -= admin->manager[Tail_idx].Runtime;
    admin->manager[Body_idx].splitted_ptr = admin->manager[Tail_idx].my_ptr;

    return true;
}
