#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"

TaskHandle_t pTask1;
TaskHandle_t pTask2;


void vTask1(void *pvParameters) 
{
    (void)pvParameters;

    while (1) {
        printf("%d : Task 1 running on Core %d\n", xTaskGetTickCount(), get_core_num());
        vTaskDelay(pdMS_TO_TICKS(500));  
    }
}

void vTask2(void *pvParameters) 
{
    (void)pvParameters;

    while (1) {
        printf("%d : Task 2 running on Core %d\n",xTaskGetTickCount(), get_core_num());
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}


int main() 
{
    stdio_init_all(); 
    TaskHandle_t pTask1;
    TaskHandle_t pTask2;
    UBaseType_t Task1_core_Affinity;
    UBaseType_t Task2_core_Affinity;
    Task1_core_Affinity = (1<<0);
    Task2_core_Affinity = (1<<1);



    xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &pTask1);
    vTaskCoreAffinitySet(pTask1,Task1_core_Affinity);

    xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &pTask2);
    vTaskCoreAffinitySet(pTask2,Task2_core_Affinity);

    vTaskStartScheduler();

    while (1) {

    }
    return 0;
}