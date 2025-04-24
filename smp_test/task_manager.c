#include "task_manager.h"


bool Periodic_Job (uint16_t Runtime, uint16_t Deadline)
{
    TickType_t end_tick = xTaskGetTickCount() + Runtime;
    TickType_t counter = 0;

    /********* CHECK DEADLINE ********/

    if(end_tick > Deadline)
    {
        return false;
    }

    /********* RUN TASK ********/

    for(int i=0; i<Runtime;i++)
    {
        counter = xTaskGetTickCount()+1;
        while(xTaskGetTickCount() < counter)
        {
            __asm volatile("nop");
        }
    }

    /********* CHECK DEADLINE ********/
    if(xTaskGetTickCount()> Deadline)
    {
        return false;
    }
    return true;
}

/***********************TASK***************************/
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
    return Light_bound;
}

void init_task_stack(task_stack* task_stack_ptr)
{
    task_stack_ptr->top = -1; //현재 stack에 들어가 있는 task 위치를 가리킴
}

float init_task(task_stack* task_stack_ptr, task_info task_list[],core_stack* core_stack_ptr)
{
    // * NOT YET : task_list를 period 짧은 순으로 정렬
    // * sorting(task_list[]);
    init_task_stack(task_stack_ptr);

    float U = get_Utilization();
    float Light_bound = get_lighttask(U);

    for(int i=0; i < NUM_OF_TASK; i++)
    {
        Push_task(&task_list[i], task_stack_ptr);

        task_info* task = task_stack_ptr->list[i];

        task->Utilization = (float) task->Runtime / (float) task->Period;

        // * Check Light or Heavy
        if(task->Utilization > Light_bound)
        {
            task->Heavy = true;
        }
        
        xTaskCreate(task->Task_Code,task->Task_Name,STACK_SIZE,NULL,
            task->priority,&(task->my_ptr));

        // * Temp : 생성된 Task를 설정된 Core에 할당
        // Assign_task(task, core_stack_ptr->list[(task->Core_Affinity >> 1)]);
    }

    return U;
}


bool Task_split(task_info* T,task_info* Tail_T, core_info* C, core_stack* core_stack_ptr,task_stack* task_stack_ptr)
{

    task_info* Body_task = T;
    task_info* Tail_task = Tail_T;

    // * Body Task Copy 
    Tail_task->Task_Name = Body_task->Task_Name;
    Tail_task->Period = Body_task->Period;
    Tail_task->priority = Body_task->priority;

    float Origin_U = Body_task->Utilization;
    float Origin_R = Body_task->Runtime;

    // * Utilization에 따른 Runtime 조정
    Body_task->Utilization = (core_stack_ptr->Utilization_Bound) - (C->Utilization);
    Body_task->Runtime = (Body_task->Utilization) * (Body_task->Period);

    Tail_task->Utilization = Origin_U - Body_task->Utilization;
    Tail_task->Runtime = Origin_R - Body_task->Runtime;


    // * Tail Task 설정 및 생성
    Tail_task->subnum = (Body_task->subnum + 1);
    Tail_task->Dependency = true;
    xTaskCreate(Tail_task->Task_Code,Tail_task->Task_Name,STACK_SIZE,NULL,
        Tail_task->priority,&(Tail_task->my_ptr));
    

    // * Body Task 재 설정
    Body_task->splitted_ptr = Tail_task->my_ptr;
    // * 쪼개진 Tail task는 UQ로 PUSH
    Push_task(Tail_task,task_stack_ptr);

    return true;
}


void Assign_task(task_info* T,core_info* C)
{
    TaskHandle_t task_ptr = T->my_ptr;
    UBaseType_t Core_num = C->Core_num;

    vTaskCoreAffinitySet(task_ptr, Core_num);
    C->Utilization += T->Utilization;
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
        printf("TASK_STACK_UNDER FLOW\n");
        return NULL;
    }

    return task_stack_ptr->list[task_stack_ptr->top--];
}

void Push_task(task_info* T, task_stack* task_stack_ptr)
{
    if(T_is_full(task_stack_ptr))
    {
        printf("TASK_STACK_OVER FLOW\n");
        return;
    }

    task_stack_ptr->list[++task_stack_ptr->top] = T;
    return;
}

void Print_task(task_info* T)
{
    printf("%s(%d) Priority:%d  Core: %d Utilization : %.3f Heavy : %d \n",T->Task_Name,T->subnum, T->priority,T->Core_Affinity, T->Utilization, T->Heavy);
}

bool simple_test(task_info* T, int num_of_Lower_T ,core_stack* core_stack_ptr,task_stack* task_stack_ptr)
{
    float sum = 0.0f;

    for(int i= num_of_Lower_T+1; i < NUM_OF_TASK ; i++)
    {
        sum += task_stack_ptr->list[i]->Utilization;
    }

    if(sum > (core_stack_ptr->top) * (core_stack_ptr->Utilization_Bound))
    {
        printf("T%d IS HEVAY BUT AFFORDABLE \n" , num_of_Lower_T+1);
        return false;
    }

    return true;
}

task_info* new_task(int* idx,task_info task_list[])
{
   
    if((*idx) >= MAX_NUM_TASKS)
    {
        printf("NO MORE NEW TASK OVERFLOW \n");
        return NULL;
    }
    task_info* new = &task_list[*idx];
    int temp_idx = *idx;
    *idx = ++temp_idx;
    
    
    return new;
}
/***********************TASK***************************/


/***********************CORE***************************/

void init_core_stack(core_stack* core_stack_ptr)
{
    core_stack_ptr->top = -1;
}

void init_core(core_stack* core_stack_ptr, core_info core_list[])
{
    init_core_stack(core_stack_ptr);
    for(int i=0; i < configNUMBER_OF_CORES; i++)
    {
        Push_core(&core_list[i],core_stack_ptr);
    }
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

core_info* Pop_core(core_stack* core_stack_ptr)
{
    if (C_is_empty(core_stack_ptr))
    {
        printf("core STACK UNDER FLOW\n");
        return NULL;
    }
    return core_stack_ptr->list[core_stack_ptr->top--];
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

core_info* get_min_core(core_stack* core_stack_ptr)
{
    int min_i = core_stack_ptr->top;
    // printf("%d \n", min_i);
    for(int i = min_i -1 ; i>=0; i--)
    {
        if(core_stack_ptr->list[i]->Utilization < core_stack_ptr->list[min_i]->Utilization)
        {
            // printf(" %f <-> %f \n",core_stack_ptr->list[i]->Utilization, core_stack_ptr->list[min_i]->Utilization);
            min_i = i;
        }
    }

    return core_stack_ptr->list[min_i];
}

/***********************CORE***************************/

// 최대공약수 계산 (유클리드 알고리즘)
uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 두 수의 최소공배수 계산
uint32_t lcm(uint32_t a, uint32_t b) {
    return (a * b) / gcd(a, b);
}

// 여러 Task의 주기 배열로 LCM 계산
uint32_t calculate_lcm(uint32_t periods[]) {
    uint32_t result = periods[0];
    for (int i = 1; i < NUM_OF_TASK; i++) {
        result = lcm(result, periods[i]);
    }
    return result;
}