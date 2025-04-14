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


void init_task(task_stack* task_stack_ptr, task_info task_list[],core_info* p_manager[])
{
    for(int i=0; i < NUM_OF_TASK; i++)
    {
        printf("%d %d\n",i,task_stack_ptr->top);
        Push_task(&task_list[i], task_stack_ptr);
        task_stack_ptr->list[i]->Utilization = (float) task_stack_ptr->list[i]->Runtime / (float) task_stack_ptr->list[i]->Period;

        xTaskCreate(task_stack_ptr->list[i]->Task_Code,task_stack_ptr->list[i]->Task_Name,STACK_SIZE,NULL,
            task_stack_ptr->list[i]->priority,&(task_stack_ptr->list[i]->my_ptr));

        Assign_task(task_stack_ptr->list[i]->my_ptr, task_stack_ptr->list[i]->Core_Affinity,
            task_stack_ptr->list[i]->Utilization, p_manager);
    }
}


// void init_task_stack(task_stack* task_stack_ptr, task_info* manager)
// {
//     // task_stack_ptr->list = manager;
//     task_stack_ptr->top = NUM_OF_TASK - 1;
// }

bool Task_split(uint8_t Body_idx, task_stack* task_stack_ptr, core_info* p_manager[])
{

    if( ++ (task_stack_ptr-> top) >= MAX_NUM_TASKS )
    {
        printf("NO MORE SPLIT \n");
        return false;
    }
    uint8_t Tail_idx = task_stack_ptr->top;

    // * Body Task Copy 
    task_stack_ptr->list[Tail_idx]->Task_Name = task_stack_ptr->list[Body_idx]->Task_Name;
    task_stack_ptr->list[Tail_idx]->Period = task_stack_ptr->list[Body_idx]->Period;
    task_stack_ptr->list[Tail_idx]->priority = task_stack_ptr->list[Body_idx]->priority;

    // * Temp : Runtime을 절반으로 쪼개서 나누어 가짐
    // * Temp : 여기서 Core를 지정할 필요는 없음
    task_stack_ptr->list[Tail_idx]->Runtime = (task_stack_ptr->list[Body_idx]->Runtime) / 2;
    task_stack_ptr->list[Tail_idx]->Core_Affinity = Core1;

    // * Tail Task 설정 및 생성
    task_stack_ptr->list[Tail_idx]->subnum = (task_stack_ptr->list[Body_idx]->subnum + 1);
    task_stack_ptr->list[Tail_idx]->Dependency = true;

    xTaskCreate(task_stack_ptr->list[Tail_idx]->Task_Code,task_stack_ptr->list[Tail_idx]->Task_Name,STACK_SIZE,NULL,
        task_stack_ptr->list[Tail_idx]->priority,&(task_stack_ptr->list[Tail_idx]->my_ptr));

    Assign_task(task_stack_ptr->list[Tail_idx]->my_ptr, task_stack_ptr->list[Tail_idx]->Core_Affinity,
        task_stack_ptr->list[Tail_idx]->Utilization, p_manager);

    // * Body Task 재 설정
    task_stack_ptr->list[Body_idx]->Runtime -= task_stack_ptr->list[Tail_idx]->Runtime;
    task_stack_ptr->list[Body_idx]->splitted_ptr = task_stack_ptr->list[Tail_idx]->my_ptr;

    return true;
}


/***********************TASK***************************/
void Assign_task(TaskHandle_t task_ptr, UBaseType_t Core_num, float Utilization, core_info* p_manager[])
{
    vTaskCoreAffinitySet(task_ptr, Core_num);
    p_manager[(Core_num >> 1)]->Utilization += Utilization;
}


void init_task_stack(task_stack* task_stack_ptr)
{
    task_stack_ptr->top = -1; //현재 stack에 들어가 있는 task 위치를 가리킴
}

bool T_is_full(task_stack* task_stack_ptr)
{
    if(task_stack_ptr->top >= (MAX_NUM_TASKS - 1) ) return true;
    return false;
}

bool T_is_empty(task_stack* task_stack_ptr)
{
    if(task_stack_ptr->top < 0) return true;
    return false;
}

task_info* Pop_task(task_stack* task_stack_ptr)
{
    if(T_is_empty(task_stack_ptr))
    {
        printf("UNDER FLOW\n");
        return NULL;
    }

    return task_stack_ptr->list[task_stack_ptr->top--];
}

void Push_task(task_info* T, task_stack* task_stack_ptr)
{
    if(T_is_full(task_stack_ptr))
    {
        printf("OVER FLOW\n");
        return;
    }

    task_stack_ptr->list[++task_stack_ptr->top] = T;
    return;
}

/***********************TASK***************************/

/***********************CORE***************************/

void init_core_stack(core_stack* core_stack_ptr)
{
    core_stack_ptr->top = -1;
}

bool C_is_full(core_stack* core_stack_ptr)
{
    if(core_stack_ptr->top >= (configNUMBER_OF_CORES -1)) return true;
    return false;
}

bool C_is_empty(core_stack* core_stack_ptr)
{
    if(core_stack_ptr->top < 0) return true;
    return false;
}

void Push_core(core_info* P, core_stack* core_stack_ptr)
{
    if(C_is_full(core_stack_ptr))
    {
        printf("core STACK OVERFLOW");
        return;
    }

    core_stack_ptr->list[++core_stack_ptr->top] = P;
    return;
}

core_info* Pop_core(core_stack* core_stack_ptr)
{
    if (C_is_empty(core_stack_ptr))
    {
        printf("core STACK UNDER FLOW\n");
        return NULL;
    }
    return core_stack_ptr->list[core_stack_ptr->top--];
}

/***********************CORE***************************/
