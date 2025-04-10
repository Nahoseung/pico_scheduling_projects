#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"


task_manager manager[3] = {
    {10,200}, // TASK 0
    {40,100}, // TASK 1
    {40,100}     // EXTRAS : TASK 2
};

void vTask0(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    uint16_t Run_tick = manager[0].Runtime; // Run_time : 10 tick -> 0.1 sec
    TickType_t Period_tick = manager[0].Period; // Period : 200 tick -> 2.0 sec

    printf("%d , %d \n",Run_tick,Period_tick);
    // uint16_t Run_tick=10;
    // TickType_t Period_tick=200;

    TickType_t Deadline;
    TickType_t LastRequestTime;

    LastRequestTime = xTaskGetTickCount();

    while (true) 
    {
        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/

        printf("%d : Task 0 execute on Core %d Deadline : %d \n", xTaskGetTickCount(), get_core_num(),Deadline);
        Periodic_Job(Run_tick,Deadline);
        printf("%d: Complete TASK0 (< %d) \n",xTaskGetTickCount(),Deadline);

        /*******************************************************************************************************/

        
        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}


void vTask1(void *pvParameters) 
{
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    uint16_t Run_tick=manager[1].Runtime; // Run_time : 40 tick -> 0.4 sec
    TickType_t Period_tick=manager[1].Period; // Period : 100 tick -> 1.0 sec
    // uint16_t Run_tick= 40;
    // TickType_t Period_tick= 100;
    printf("%d , %d \n",Run_tick,Period_tick);

    TickType_t Deadline;
    TickType_t LastRequestTime;

    LastRequestTime = xTaskGetTickCount();

    while (true) 
    {
        Deadline = LastRequestTime + Period_tick; // 최신 요청 기준 Deadline 업데이트

        /*******************************************************************************************************/

        printf("%d : Task 1 execute on Core %d Deadline : %d \n", xTaskGetTickCount(), get_core_num(),Deadline);
        Periodic_Job(Run_tick,Deadline);
        printf("%d: Complete TASK1 (< %d) \n",xTaskGetTickCount(),Deadline);

        /*******************************************************************************************************/

        
        vTaskDelayUntil(&LastRequestTime, Period_tick);  
    }
}



int main() 
{
    stdio_init_all(); 

    TaskHandle_t pTask0;
    TaskHandle_t pTask1;

    UBaseType_t Task0_core_Affinity;
    UBaseType_t Task1_core_Affinity;

    Task0_core_Affinity = (1<<0);
    Task1_core_Affinity = (1<<1);

    sleep_ms(5000);
    printf("START TASKING\n");

    xTaskCreate(vTask0, "Task 0", configMINIMAL_STACK_SIZE, NULL, 1, &pTask0);
    vTaskCoreAffinitySet(pTask0,Task0_core_Affinity);

    /*
     *       vTaskSplit() {}: Runtime 40 -> 20 으로 외부에서 수정 -> Task Manager 도입
     */

    xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &pTask1);
    vTaskCoreAffinitySet(pTask1,Task1_core_Affinity);

    /*
     * ******************** SPA2  ************************
     */

    vTaskStartScheduler(); //Tick Count 시작

    while (true);

    return 0;
}