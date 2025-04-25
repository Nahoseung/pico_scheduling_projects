#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define sync_R 10  // * vTaskDelay(pdMS_TO_TICKS(sync_R));  delay 1tick
#define monitor 1
#define debugging
#define SPA2 0

void vTask0(void* pvParameters);
void vTask1(void* pvParameters);
void vTask2(void* pvParameters);
void vTask3(void* pvParameters);
void vTask4(void* pvParameters);
void vTask5(void* pvParameters);


task_info task_list [MAX_NUM_TASKS] =
{
        /* Code /  Name  / Run / Period / Core / Priority / Subnum / my_ptr / splitted_ptr / Dependency / Heavy */

        {vTask0, "TASK 0",  2, 5,  Core0, 5,1, NULL, NULL, false,false}, 
        {vTask1, "TASK 1",  3, 10,  Core1, 4,1, NULL, NULL, false,false}, 
        {vTask2, "TASK 2",  38, 50,  Core1, 3,1, NULL, NULL, false,false},
        {vTask3, "TASK 3",  1, 100,  Core0, 2,1, NULL, NULL, false,false},
        {vTask4, "TASK 4",  1, 100,  Core0, 1,1, NULL, NULL,  false,false},
        // * Extra Tasks 
        {vTask5, "Extra T",   0,   0,  0, 1,1, NULL, NULL,  false,false}
};
task_stack task_manager;
task_stack UQ;



core_info core_list[configNUMBER_OF_CORES] = 
{
    {Core0, 0.0,false},
    {Core1, 0.0,false}
};
core_stack core_manager;
core_stack pre_assigned_core;

bool schedulable = true;
void MonitorTask(void*pvParameters)
{
   
    
    uint32_t period_list[NUM_OF_TASK] ;

    for(int i=0;i <NUM_OF_TASK;i++)
    {
        period_list[i]= task_list[i].Period;
    }
    uint32_t lcm_ticks = calculate_lcm(period_list);
    printf("MONITOR ACTIVATED LCM: %d\n",lcm_ticks);
    
    TickType_t start = xTaskGetTickCount();
    vTaskDelay(lcm_ticks + 1);

    if (!schedulable)
    {
        printf("NOT SCHEDULABLE\n");
    }

    else
    {
        printf("ALL TASKS SCHEDULABLE\n");       
    }

    for(int i=0; i<MAX_NUM_TASKS;i++)
    {
        vTaskDelete(task_list[i].my_ptr);
    }

    vTaskDelete(NULL);


}

int main() 
{
    stdio_init_all(); 

    sleep_ms(5000);

    // printf("START KERNEL at : CORE %d \n",get_core_num());

    #if monitor
    xTaskCreate(MonitorTask,"MONITOR",STACK_SIZE,NULL,10,NULL);
    #endif

    //* Core ptr을 STACK에 PUSH
    init_core(&core_manager,core_list);

    //* TASK들을 STACK에 PUSH
    float Utilization_Bound = init_task(&task_manager,task_list,&core_manager);
    int curr_idx = NUM_OF_TASK;

    core_manager.Utilization_Bound = Utilization_Bound;
    pre_assigned_core.Utilization_Bound = Utilization_Bound;

    //* UQ, PQ_pre stack들을 초기화
    init_core_stack(&pre_assigned_core);
    init_task_stack(&UQ);


    /******************* SPA2 ALGORITHM **************************/
  
    #if SPA2
    /* PRE-ASSIGN */
    for(int i=0; i < NUM_OF_TASK; i++)
    {
        task_info* T= task_manager.list[i];
        if(T->Heavy&&simple_test(T,i,&core_manager,&task_manager))
        {
            core_info* C = Pop_core(&core_manager);
            // printf("T: %s -> C: %d (%f) \n", T->Task_Name,C->Core_num,C->Utilization);
            Assign_task(T,C);
            C->pre_assigned = true;
            Push_core(C,&pre_assigned_core);
        }
        else
        {
            Push_task(T,&UQ);
        }
    }

    /* NORMAL-ASSIGN */
    while(!T_is_empty(&UQ))
    {
        /* SELCECT CORE */
        core_info* min_C = get_min_core(&core_manager);
        task_info* T = Pop_task(&UQ);

        
        if(min_C->Utilization >= Utilization_Bound)
        {
            min_C = Pop_core(&pre_assigned_core);
            if(min_C->Utilization >= Utilization_Bound)
            {
                printf("ALL PROCESSORS FULLY UTILIZED \n");
                fflush(stdout);
                break;
            }
        }

        // printf("T: %s -> C: %d (%f) \n", T->Task_Name,min_C->Core_num,min_C->Utilization);

        if(min_C->Utilization + T->Utilization <= Utilization_Bound)
        {
            Assign_task(T,min_C);
        }

        /* SPLIT */
        else
        {
            task_info* Tail_T = new_task(&curr_idx,task_list);
            Task_split(T,Tail_T,min_C,&core_manager,&UQ);
            Assign_task(T,min_C);
        }

        if(min_C->pre_assigned)
        {
            Push_core(min_C,&pre_assigned_core);
        }
    }
    #else

    while(!T_is_empty(&task_manager))
    {
        /* SELCECT CORE */
        core_info* min_C = get_min_core(&core_manager);
        task_info* T = Pop_task(&task_manager);

        if(min_C->Utilization >= Utilization_Bound)
        {
            printf("ALL PROCESSORS FULLY UTILIZED \n");
            fflush(stdout);
            break;
        }
        
        // printf("T: %s -> C: %d (%f) \n", T->Task_Name,min_C->Core_num,min_C->Utilization);
        if(min_C->Utilization + T->Utilization <= Utilization_Bound)
        {
            Assign_task(T,min_C);
        }
        
        /* SPLIT */
        else
        {
            task_info* Tail_T = new_task(&curr_idx,task_list);
            Task_split(T,Tail_T,min_C,&core_manager,&task_manager);
            Assign_task(T,min_C);
        }
    }

    #endif

    printf("Core 0 Utilization : %.3f, Core 1 Utilization : %.3f \n" , core_manager.list[0]->Utilization, core_manager.list[1]->Utilization);
    
    vTaskStartScheduler(); 
    

    while (true);

    return 0;
}


/************************************************* PERIODIC TASKS ******************************************************/

void run_task(task_info Task, TickType_t* LastRequestTime,TickType_t Deadline, bool flag)
{
    if(Task.Dependency)
    {
        printf("%d: %s(%d) Suspended \n",xTaskGetTickCount(),Task.Task_Name, Task.subnum);
        vTaskSuspend(Task.my_ptr);
    }

    Deadline = *(LastRequestTime) + Task.Period; // 최신 요청 기준 Deadline 업데이트

    /* RUN */
    printf("%d : %s(%d) execute on Core %d Deadline : %d\n", xTaskGetTickCount(), Task.Task_Name,Task.subnum,get_core_num(),Deadline);
    flag = Periodic_Job(Task.Runtime,Deadline);


    /* OVERFLOW CHECK*/
    if(!flag)
    {
        printf("%d: OVERFLOW %s(%d) at Core %d\n",xTaskGetTickCount(),Task.Task_Name,Task.subnum,get_core_num());
        printf("GOOD BYE Core %d", get_core_num());
        schedulable=false;

        // * NEVER REACH
        while(true);
    }

    if (Task.splitted_ptr != NULL)
    {

        vTaskResume(Task.splitted_ptr);
    }

    printf("%d: Complete %s(%d) (< %d)\n",xTaskGetTickCount(),Task.Task_Name,Task.subnum,Deadline);
    vTaskDelayUntil(LastRequestTime, Task.Period);  
}


void vTask0(void *pvParameters) 
{ 
    /*************************************/
    static const uint8_t task_num= 0;
    task_info Task = task_list[task_num];
    /*************************************/
    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
    
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}


void vTask1(void *pvParameters) 
{
    /*************************************/
    static const uint8_t task_num= 1;
    task_info Task = task_list[task_num];
    /*************************************/
    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
    
    
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}

void vTask2(void *pvParameters) 
{
    /*************************************/
    static const uint8_t task_num= 2;
    task_info Task = task_list[task_num];
    /*************************************/

    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
     
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}


void vTask3(void *pvParameters) 
{ 
    /*************************************/
    static const uint8_t task_num= 3;
    task_info Task = task_list[task_num];
    /*************************************/

    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
    
    
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}

void vTask4(void *pvParameters) 
{ 
    /*************************************/
    static const uint8_t task_num= 4;
    task_info Task = task_list[task_num];
    /*************************************/

    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
    
    
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}

void vTask5(void *pvParameters) 
{ 
    /*************************************/
    static const uint8_t task_num= 5;
    task_info Task = task_list[task_num];
    /*************************************/

    TickType_t Deadline;
    TickType_t LastRequestTime= xTaskGetTickCount();
    bool flag=false;


    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task.Task_Name, Task.subnum, Task.Runtime, Task.Period, Task.Utilization,Task.priority,get_core_num());

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick for sync
    
    while (true) 
    {
        run_task(Task,&LastRequestTime,Deadline,flag);
    }
}




