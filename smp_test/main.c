#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define sync_R 10

void vTask0(void* pvParameters);
void vTask1(void* pvParameters);
void vTask2(void* pvParameters);

#if BusyWaitting
    bool Task0_end = false; 
#endif


task_info task_list [MAX_NUM_TASKS] ={

        /* Code /  Name  / Run / Period / Core / Priority / Subnum / my_ptr / splitted_ptr / Dependency */

        {vTask0, "TASK 0",  40, 100, Core0, 2,1, NULL, NULL, false}, // TASK 0
        {vTask1, "TASK 1",  40, 100, Core1, 2,1, NULL, NULL, false}, // TASK 1
        // {vTask2,0,}
        {vTask2, "TASK 2",  20, 100, Core1, 1,1, NULL, NULL, false}  // TASK 2
};
task_stack task_manager;

core_info P0 = {Core0,0.0};
core_info P1 = {Core1,0.0};
core_info* P_manager[configNUMBER_OF_CORES] = {&P0, &P1};  //[2]


int main() 
{
    stdio_init_all(); 

    sleep_ms(5000);
    printf("START KERNEL at : %d \n",get_core_num());

    // * STACK 초기화
    init_task_stack(&task_manager); 
    printf("INIT STACK %d \n",get_core_num());  

    // * TASK들을 STACK에 PUSH
    init_task(&task_manager,task_list,P_manager);
    printf("INIT TASK %d \n", get_core_num());  

    // * POP TEST
    // Print_task(Pop_task(&task_manager)); 

    // Task_split(0,&admin);
    
    // * Dependency TEST
    #if !BusyWaitting
    task_list[0].splitted_ptr = task_list[1].my_ptr;
    task_list[1].Dependency = true;
    #endif

    printf("Core 0 Utilization : %.3f, Core 1 Utilization : %.3f \n" , P_manager[Core0 >>1]->Utilization, P_manager[Core1>>1]->Utilization);
    fflush(stdout);
    // sleep_ms(1000);
    /*
     * ******************** SPA2  ************************
     */

    vTaskStartScheduler(); //Tick Count 시작

    while (true);

    return 0;
}

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
    
    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());
    LastRequestTime = xTaskGetTickCount();
    

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while (true) 
    {

        if(Dependency)
        {
            printf("%s suspending something\n",Task_name);
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
    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());
    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick
    

    while (true) 
    {
        if(Dependency)
        {
            printf("%s suspending something\n",Task_name);
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

    printf("%s(%d) (%d , %d) Utilization : %.3f priority: %d at : %d \n", Task_name, subnum, Run_tick, Period_tick,task_list[task_num].Utilization,task_list[task_num].priority,get_core_num());

    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick
    
    while (true) 
    {
        if(Dependency)
        {
            printf("%s suspending something\n",Task_name);
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


