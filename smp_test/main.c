#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define BusyWaitting 0
#define sync_R 10

void vTask0(void* pvParameters);
void vTask1(void* pvParameters);
void vTask2(void* pvParameters);

#if BusyWaitting
    bool Task0_end = false; 
#endif

administrator admin;

task_info manager [MAX_NUM_TASKS] ={

        /* Code /  Name  / Run / Period / Core / Priority / Subnum / my_ptr / splitted_ptr / Dependency */

        {vTask0, "TASK 0",  40, 100, Core0, 2,1, NULL, NULL, false}, // TASK 0
        {vTask1, "TASK 1",  40, 100, Core1, 1,1, NULL, NULL, false}, // TASK 1
        {vTask2,0,}
        // {vTask2, "TASK 0",  20, 100, Core1, 2,1, NULL, NULL, false}  // TASK 2
};


void vTask0(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 0;
    /*******************************/

    uint16_t Run_tick = manager[task_num].Runtime; // Run_time : 10 tick -> 0.1 sec
    TickType_t Period_tick = manager[task_num].Period; // Period : 200 tick -> 2.0 sec
    const char* Task_name = manager[task_num].Task_Name;
    uint8_t subnum = manager[task_num].subnum;
    TaskHandle_t my_ptr = manager[task_num].my_ptr;
    TaskHandle_t splitted_ptr = manager[task_num].splitted_ptr;
    bool Dependency = manager[task_num].Dependency;

    printf("%s(%d) (%d , %d) priority: %d at : 0x%d \n", Task_name, subnum, Run_tick, Period_tick,manager[task_num].priority,manager[task_num].my_ptr);

    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;
    
    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while (true) 
    {
        #if BusyWaitting
        Task0_end = false;
        #else 
        if(Dependency)
        {
            vTaskSuspend(my_ptr);
        }
        #endif

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

    uint16_t Run_tick = manager[task_num].Runtime; // Run_time : 10 tick -> 0.1 sec
    TickType_t Period_tick = manager[task_num].Period; // Period : 200 tick -> 2.0 sec
    const char* Task_name = manager[task_num].Task_Name;
    uint8_t subnum = manager[task_num].subnum;
    TaskHandle_t my_ptr = manager[task_num].my_ptr;
    TaskHandle_t splitted_ptr = manager[task_num].splitted_ptr;
    bool Dependency = manager[task_num].Dependency;

    printf("%s(%d) (%d , %d) priority %d at : 0x%d \n", Task_name, subnum, Run_tick, Period_tick,manager[task_num].priority,manager[task_num].my_ptr);

    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;
    
    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while (true) 
    {
        #if BusyWaitting
        Task0_end = false;
        #else 
        if(Dependency)
        {
            vTaskSuspend(my_ptr);
        }
        #endif

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

    uint16_t Run_tick = manager[task_num].Runtime; // Run_time : 10 tick -> 0.1 sec
    TickType_t Period_tick = manager[task_num].Period; // Period : 200 tick -> 2.0 sec
    const char* Task_name = manager[task_num].Task_Name;
    uint8_t subnum = manager[task_num].subnum;
    TaskHandle_t my_ptr = manager[task_num].my_ptr;
    TaskHandle_t splitted_ptr = manager[task_num].splitted_ptr;
    bool Dependency = manager[task_num].Dependency;

    printf("%s(%d) (%d , %d) priority %d at : 0x%d \n", Task_name, subnum, Run_tick, Period_tick,manager[task_num].priority,manager[task_num].my_ptr);

    TickType_t Deadline;
    TickType_t LastRequestTime;
    bool flag=false;
    
    LastRequestTime = xTaskGetTickCount();

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while (true) 
    {
        #if BusyWaitting
        Task0_end = false;
        #else 
        if(Dependency)
        {
            vTaskSuspend(my_ptr);
        }
        #endif

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


int main() 
{
    stdio_init_all(); 

    sleep_ms(5000);
    printf("START KERNEL \n");
    init_admin(&admin,manager);

    
    init_task(&admin);

    Task_split(0,&admin);
    

    // #if !BusyWaitting
    // manager[0].splitted_ptr = manager[1].my_ptr;
    // manager[1].Dependency = true;
    // #endif

    /*
     * ******************** SPA2  ************************
     */

    vTaskStartScheduler(); //Tick Count 시작

    while (true);

    return 0;
}