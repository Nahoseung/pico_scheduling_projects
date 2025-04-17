#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define sync_R 10

void vTask0(void* pvParameters);
void vTask1(void* pvParameters);
void vTask2(void* pvParameters);

task_info task_list [MAX_NUM_TASKS] =
{

        /* Code /  Name  / Run / Period / Core / Priority / Subnum / my_ptr / splitted_ptr / Dependency / Heavy */

        {vTask0, "TASK 0",  40, 100, Core0, 2,1, NULL, NULL, false,false}, // TASK 0
        {vTask1, "TASK 1",  50, 100, Core1, 2,1, NULL, NULL, false,false}, // TASK 1
        // {vTask2,0,}
        {vTask2, "TASK 2",  30, 100, Core1, 1,1, NULL, NULL, false,false}  // TASK 2

};
task_stack task_manager;
task_stack Normal_task;

core_info core_list[configNUMBER_OF_CORES] = 
{
    {Core0, 0.0,false},
    {Core1, 0.0,false}
};
core_stack core_manager;
core_stack pre_assigned_core;

int main() 
{
    stdio_init_all(); 

    sleep_ms(5000);
    printf("START KERNEL at : CORE %d \n",get_core_num());

    // * Core ptr을 STACK에 PUSH
    init_core(&core_manager,core_list);

    // * TASK들을 STACK에 PUSH
    float Utilization_Bound = init_task(&task_manager,task_list,&core_manager);

    // * UQ, PQ_pre stack들을 초기화
    init_core_stack(&pre_assigned_core);
    init_task_stack(&Normal_task);
    printf("Core 0 Utilization : %.3f, Core 1 Utilization : %.3f \n" , core_manager.list[0]->Utilization, core_manager.list[1]->Utilization);
    // * POP TEST
    // task_info* tempT = Pop_task(&task_manager); 
    // Push_task(tempT,&Normal_task);
    // Print_task(&task_list[1]);

        // Task_split(0,&admin);
    
    // * Dependency TEST
    task_list[0].splitted_ptr = task_list[1].my_ptr;
    task_list[1].Dependency = true;

    // * POP TEST
    core_info*  temp = get_min_core(&core_manager);
    printf("Min core %d  Utilization %.3f\n", (temp->Core_num>>1),temp->Utilization);

    // bool flag = simple_test(&task_list[0],0,1,&task_manager,Utilization_Bound);
    // printf("task 0 simple test result : %d \n",flag);

    // temp = Pop_core(&core_manager);
    // Push_core(temp,&pre_assigned_core);
    
    // printf("Core 0 Utilization : %.3f, Core 1 Utilization : %.3f \n" , core_manager.list[core_manager.top]->Utilization, pre_assigned_core.list[pre_assigned_core.top]->Utilization);


    /*
     * ******************** SPA2  ************************
     */

    vTaskStartScheduler(); //Tick Count 시작

    while (true);

    return 0;
}


/************************************************* PERIODIC TASKS ******************************************************/


void vTask0(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 0;
    /*******************************/

    uint16_t Run_tick = task_list[task_num].Runtime; 
    TickType_t Period_tick = task_list[task_num].Period; 
    const char* Task_name = task_list[task_num].Task_Name;
    uint8_t subnum = task_list[task_num].subnum;
    TaskHandle_t my_ptr = task_list[task_num].my_ptr;
    TaskHandle_t splitted_ptr = task_list[task_num].splitted_ptr;
    bool Dependency = task_list[task_num].Dependency;

   

    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;
    
    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());
    LastRequestTime = xTaskGetTickCount();
    

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while (true) 
    {

        if(Dependency)
        {
            printf("%s Suspended \n",Task_name);
            vTaskSuspend(my_ptr);
        }

        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/
        printf("%d : %s(%d) execute on Core %d Deadline : %d \n", xTaskGetTickCount(), Task_name,subnum,get_core_num(),Deadline);
        flag = Periodic_Job(Run_tick,Deadline);


        /***************CHECK OVERFLOW****************/
        if(!flag)
        {
            printf("OVERFLOW %s(%d) at Core %d\n",Task_name,subnum,get_core_num());
            printf("GOOD BYE Core %d", get_core_num());
            return;
        }

        if (splitted_ptr != NULL)
        {

            vTaskResume(splitted_ptr);
        }

        #if BusyWaitting
        Task0_end = true;
        #else
        vTaskResume(splitted_ptr);
        #endif

        printf("%d: Complete %s(%d) (< %d) \n",xTaskGetTickCount(),Task_name,subnum,Deadline);
        /*******************************************************************************************************/

        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}


void vTask1(void *pvParameters) 
{
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 1;
    /*******************************/

    uint16_t Run_tick = task_list[task_num].Runtime; 
    TickType_t Period_tick = task_list[task_num].Period; 
    const char* Task_name = task_list[task_num].Task_Name;
    uint8_t subnum = task_list[task_num].subnum;
    TaskHandle_t my_ptr = task_list[task_num].my_ptr;
    TaskHandle_t splitted_ptr = task_list[task_num].splitted_ptr;
    bool Dependency = task_list[task_num].Dependency;

    

    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;
    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());
    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick
    

    while (true) 
    {
        if(Dependency)
        {
            printf("%s Suspended \n",Task_name);
            vTaskSuspend(my_ptr);
        }

        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/
        printf("%d : %s(%d) execute on Core %d Deadline : %d \n", xTaskGetTickCount(), Task_name,subnum,get_core_num(),Deadline);
        flag = Periodic_Job(Run_tick,Deadline);


        /***************CHECK OVERFLOW****************/
        if(!flag)
        {
            printf("OVERFLOW %s(%d) at Core %d\n",Task_name,subnum,get_core_num());
            printf("GOOD BYE Core %d", get_core_num());
            return;
        }

        if (splitted_ptr != NULL)
        {

            vTaskResume(splitted_ptr);
        }

        #if BusyWaitting
        Task0_end = true;
        #else
        vTaskResume(splitted_ptr);
        #endif

        printf("%d: Complete %s(%d) (< %d) \n",xTaskGetTickCount(),Task_name,subnum,Deadline);
        /*******************************************************************************************************/

        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}

void vTask2(void *pvParameters) 
{
/****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 2;
    /*******************************/

    uint16_t Run_tick = task_list[task_num].Runtime; 
    TickType_t Period_tick = task_list[task_num].Period; 
    const char* Task_name = task_list[task_num].Task_Name;
    uint8_t subnum = task_list[task_num].subnum;
    TaskHandle_t my_ptr = task_list[task_num].my_ptr;
    TaskHandle_t splitted_ptr = task_list[task_num].splitted_ptr;
    bool Dependency = task_list[task_num].Dependency;


    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;

    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : CORE %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());

    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick
    
    while (true) 
    {
        if(Dependency)
        {
            printf("%s Suspended \n",Task_name);
            vTaskSuspend(my_ptr);
        }

        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/
        printf("%d : %s(%d) execute on Core %d Deadline : %d \n", xTaskGetTickCount(), Task_name,subnum,get_core_num(),Deadline);
        flag = Periodic_Job(Run_tick,Deadline);


        /***************CHECK OVERFLOW****************/
        if(!flag)
        {
            printf("OVERFLOW %s(%d) at Core %d\n",Task_name,subnum,get_core_num());
            printf("GOOD BYE Core %d", get_core_num());
            return;
        }

        if (splitted_ptr != NULL)
        {

            vTaskResume(splitted_ptr);
        }

        #if BusyWaitting
        Task0_end = true;
        #else
        vTaskResume(splitted_ptr);
        #endif

        printf("%d: Complete %s(%d) (< %d) \n",xTaskGetTickCount(),Task_name,subnum,Deadline);
        /*******************************************************************************************************/

        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}


