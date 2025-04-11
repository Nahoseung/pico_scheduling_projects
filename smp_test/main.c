#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define HARD 0
#define BusyWaitting 0
void vTask0(void* pvParameters);
void vTask1(void* pvParameters);

TaskHandle_t pTask0 = NULL;
TaskHandle_t pTask1 = NULL;

TaskHandle_t ptr_pool[MAX_NUM_TASKS];
task_manager manager [MAX_NUM_TASKS] ={

        /* Code /  Name  / Run / Period / Core Affinity / my_ptr / splitted_ptr / Dependency */

         {vTask0,"TASK0",  10, 200, Core0}, // TASK 0
         {vTask1,"TASK1",  40, 100, Core1}, // TASK 1
         {NULL,}

};



void vTask0(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/
    static const uint8_t priority = 0;

    #if(HARD)
    uint16_t Run_tick= 10;
    TickType_t Period_tick= 200;
    #else
    uint16_t Run_tick = manager[priority].Runtime; // Run_time : 10 tick -> 0.1 sec
    TickType_t Period_tick = manager[priority].Period; // Period : 200 tick -> 2.0 sec
    const char* Task_name = manager[priority].Task_Name;
    #endif

    printf("%s (%d , %d)  \n",Task_name,Run_tick,Period_tick);

    TickType_t Deadline;
    TickType_t LastRequestTime;

    LastRequestTime = xTaskGetTickCount();

    while (true) 
    {
        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/

    
        #if(HARD)    
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("OVERFLOW \n ");
            sleep_ms(5000);
            return ;
        }
        printf("%d : Task 0 execute on Core %d Deadline : %d \n", xTaskGetTickCount(), get_core_num(),Deadline);
        /********* RUN TASK ********/
        for(int i=0;i<Run_tick;i++)
        {
            // printf("0");
            sleep_ms(10); // 1 Tick
        }
        // printf(" | \n");
        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW : MISS DEADLINE\n");
            sleep_ms(5000);
            return ;
        }


        #else
        printf("%d : %s execute on Core %d Deadline : %d \n", xTaskGetTickCount(), Task_name,get_core_num(),Deadline);
        Periodic_Job(Run_tick,Deadline);
        #endif

        printf("%d: Complete %s (< %d) \n",xTaskGetTickCount(),Task_name,Deadline);

        /*******************************************************************************************************/

        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }

}


void vTask1(void *pvParameters) 
{
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/
    static const uint8_t priority = 1;

    #if(HARD)
    uint16_t Run_tick= 40;
    TickType_t Period_tick= 100;
    #else
    uint16_t Run_tick = manager[priority].Runtime; // Run_time : 40 tick -> 0.4 sec
    TickType_t Period_tick = manager[priority].Period; // Period : 100 tick -> 1.0 sec
    const char* Task_name = manager[priority].Task_Name;
    #endif

    printf("%s (%d , %d) \n",Task_name,Run_tick,Period_tick);

    TickType_t Deadline;
    TickType_t LastRequestTime;

    LastRequestTime = xTaskGetTickCount();

    while (true) 
    {
        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/

        
        #if(HARD)
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("OVERFLOW \n ");
            sleep_ms(5000);
            return ;
        } 
        printf("%d : Task 1 execute on Core %d Deadline : %d \n", xTaskGetTickCount(), get_core_num(),Deadline);
        /********* RUN TASK ********/
        for(int i=0;i<Run_tick;i++)
        {
            // printf("1");
            sleep_ms(10); // 1 Tick
        }
        // printf("| \n");
        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW : MISS DEADLINE\n");
            sleep_ms(5000);
            return ;
        }

        #else
        printf("%d : %s execute on Core %d Deadline : %d \n", xTaskGetTickCount(), Task_name,get_core_num(),Deadline);
        Periodic_Job(Run_tick,Deadline);
        #endif

        printf("%d: Complete %s (< %d) \n",xTaskGetTickCount(),Task_name,Deadline);

        /*******************************************************************************************************/
        
        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}



int main() 
{
    stdio_init_all(); 

    sleep_ms(5000);
    printf("START TASKING %d\n", HARD);

    #if(HARD)
    UBaseType_t Task0_core_Affinity = Core0;
    UBaseType_t Task1_core_Affinity = Core1;
    xTaskCreateAffinitySet(vTask0, "Task 0", 256, NULL, 2,Task0_core_Affinity,NULL);
    xTaskCreateAffinitySet(vTask1, "Task 1", 256, NULL, 1, Task1_core_Affinity,NULL);
    #else
    init(manager);
    #endif

    /*
     *       vTaskSplit() {}: Runtime 40 -> 20 으로 외부에서 수정 -> Task Manager 도입
     */

    manager[1].Runtime = 20;


    /*
     * ******************** SPA2  ************************
     */

    vTaskStartScheduler(); //Tick Count 시작

    while (true);

    return 0;
}