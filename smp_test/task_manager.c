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


void init_task(task_stack* task_stack_ptr, task_info task_list[],core_stack* core_stack_ptr)
{

    init_task_stack(task_stack_ptr);
    float U = get_Utilization();
    float Light_bound = get_lighttask(U);

    for(int i=0; i < NUM_OF_TASK; i++)
    {
        Push_task(&task_list[i], task_stack_ptr);

        task_stack_ptr->list[i]->Utilization = (float) task_stack_ptr->list[i]->Runtime / (float) task_stack_ptr->list[i]->Period;

        // * Check Light or Heavy
        if(task_stack_ptr->list[i]->Utilization > Light_bound)
        {
            task_stack_ptr->list[i]->Heavy = true;
        }

        xTaskCreate(task_stack_ptr->list[i]->Task_Code,task_stack_ptr->list[i]->Task_Name,STACK_SIZE,NULL,
            task_stack_ptr->list[i]->priority,&(task_stack_ptr->list[i]->my_ptr));
        // * Temp : 생성된 Task를 설정된 Core에 할당
        Assign_task(task_stack_ptr->list[i]->my_ptr, task_stack_ptr->list[i]->Core_Affinity,
            task_stack_ptr->list[i]->Utilization, core_stack_ptr);
    }
}


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
    
    // * Temp : 생성된 Task를 설정된 Core에 할당
    // Assign_task(task_stack_ptr->list[Tail_idx]->my_ptr, task_stack_ptr->list[Tail_idx]->Core_Affinity,
    //     task_stack_ptr->list[Tail_idx]->Utilization, p_manager);

    // * Body Task 재 설정
    task_stack_ptr->list[Body_idx]->Runtime -= task_stack_ptr->list[Tail_idx]->Runtime;
    task_stack_ptr->list[Body_idx]->splitted_ptr = task_stack_ptr->list[Tail_idx]->my_ptr;

    return true;
}


/***********************TASK***************************/

void Assign_task(TaskHandle_t task_ptr, UBaseType_t Core_num, float Utilization, core_stack* core_stack_ptr)
{
    vTaskCoreAffinitySet(task_ptr, Core_num);
    core_stack_ptr->list[(Core_num >> 1)]->Utilization += Utilization;
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

void Print_task(task_info* T)
{
    printf("%s(%d) Priority:%d  Core: %d Utilization : %.3f Heavy : %d \n",T->Task_Name,T->subnum, T->priority,T->Core_Affinity, T->Utilization, T->Heavy);
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

void init_core(core_stack* core_stack_ptr, core_info core_list[])
{
    init_core_stack(core_stack_ptr);
    for(int i=0; i<configNUMBER_OF_CORES;i++)
    {
        Push_core(&core_list[i],core_stack_ptr);
    }
}
/***********************CORE***************************/

float get_Utilization()
{
    float n = NUM_OF_TASK;
    float U = n * (pow(2.0, 1.0 / n) - 1);
    printf("For %d tasks Utilization Bound is :  %.4f \n",NUM_OF_TASK, U);
    return U;
}
float get_lighttask(float U)
{
    float Light_bound = U/(1+U);
    printf("For %d tasks Light_Bound is :  %.4f \n",NUM_OF_TASK, Light_bound);
    return Light_bound;
}