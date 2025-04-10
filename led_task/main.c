#include "pico/stdlib.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define sync_R 10


void task1(void* pvParameter)
{
    const uint16_t Run_tick=10; 
    const uint16_t Run_time = Run_tick * 10;

    const TickType_t Period=50; 
    TickType_t LastRequestTime;
    TickType_t Deadline;

    LastRequestTime = xTaskGetTickCount();
    printf("%d: TASK 1 First Request\n",LastRequestTime);

    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while(true)
    {
        Deadline = LastRequestTime + Period;

        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("TASK 1 OVERFLOW \n");
            sleep_ms(5000);
            return;
        }

        /********* RUN TASK ********/
        printf("%d : EXECUTION TASK 1 \n", xTaskGetTickCount());
        for(int i=0;i<Run_tick;i++)
        {
            sleep_ms(10);
        } 

        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW TASK1: MISS ITS DEADLINE\n");
            sleep_ms(5000);
            return;
        }

        printf("%d: Complete TASK1 (< %d) \n",xTaskGetTickCount(),Deadline);
        vTaskDelayUntil(&LastRequestTime, Period);
    }
}

void task2(void* pvParameter)
{
    const uint16_t Run_tick=40;
    const uint16_t Run_time = Run_tick * 10;

    const TickType_t Period=100;
    TickType_t LastRequestTime;
    TickType_t Deadline;

    LastRequestTime = xTaskGetTickCount();
    printf("%d: TASK 2 First Request\n",LastRequestTime);
    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while(true)
    {
        Deadline = LastRequestTime + Period;
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("TASK 2 OVERFLOW \n ");
            sleep_ms(5000);
            return;
        }

        /********* RUN TASK ********/
        printf("%d : EXECUTION TASK 2 \n", xTaskGetTickCount());
        for(int i=0;i<Run_tick;i++)
        {
            sleep_ms(10);
        }

        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW TASK2: MISS DEADLINE\n");
            sleep_ms(5000);
            return;
        }

        printf("%d: Complete TASK2 (< %d) \n",xTaskGetTickCount(),Deadline);
        vTaskDelayUntil(&LastRequestTime, Period);
    }
}

void task3(void* pvParameter)
{
    const uint16_t Run_tick=10; //0.25 sec 동안 수행
    const uint16_t Run_time = Run_tick * 10;

    const TickType_t Period=60; //0.7 sec 주기로 수행
    TickType_t LastRequestTime;
    TickType_t Deadline;

    LastRequestTime = xTaskGetTickCount();
    printf("%d: TASK 3 First Request\n",LastRequestTime);
    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while(true)
    {
        Deadline = LastRequestTime + Period;
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("TASK 3 OVERFLOW \n");
            sleep_ms(5000);
            return;
        }

        /********* RUN TASK ********/
        printf("%d : EXECUTION TASK 3 \n", xTaskGetTickCount());
        for(int i=0;i<Run_tick;i++)
        {
            sleep_ms(10);
        } 

        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW TASK3: MISS ITS DEADLINE\n");
            sleep_ms(5000);
            return;
        }


        printf("%d: Complete TASK3 (< %d) \n",xTaskGetTickCount(),Deadline);
        vTaskDelayUntil(&LastRequestTime, Period);
    }
}

void task4(void* pvParameter)
{
    const uint16_t Run_tick=10;
    const uint16_t Run_time = Run_tick * 10;

    const TickType_t Period=200;
    TickType_t LastRequestTime;
    TickType_t Deadline;

    LastRequestTime = xTaskGetTickCount();
    printf("%d: TASK 4 First Request\n",LastRequestTime);
    vTaskDelay(pdMS_TO_TICKS(sync_R)); // delay 1tick

    while(true)
    {
        Deadline = LastRequestTime + Period;
        if(xTaskGetTickCount() + Run_tick >= Deadline)
        {
            printf("TASK 4 OVERFLOW \n ");
            sleep_ms(5000);
            return;
        }

        /********* RUN TASK ********/
        printf("%d : EXECUTION TASK 4 \n", xTaskGetTickCount());
        for(int i=0;i<Run_tick;i++)
        {
            sleep_ms(10);
        }

        /********* CHECK DEADLINE ********/
        if(xTaskGetTickCount()> Deadline)
        {
            printf("OVERFLOW TASK4: MISS DEADLINE\n");
            sleep_ms(5000);
            return;
        }


        printf("%d: Complete TASK4 (< %d) \n",xTaskGetTickCount(),Deadline);
        vTaskDelayUntil(&LastRequestTime, Period);
    }
}


int main() 
{
    stdio_usb_init();
    sleep_ms(5000);

    xTaskCreate(task1, "task1", 256, NULL, 4, NULL);
    xTaskCreate(task2, "task2", 256, NULL, 3, NULL);
    xTaskCreate(task3, "task3", 256, NULL, 2, NULL);
    xTaskCreate(task4, "task4", 256, NULL, 1, NULL);
    

    printf("LED TASK PROGRAM START\n");
    printf("Per Tick : %d ms\n", portTICK_PERIOD_MS);  // tick 0.01 sec ( ms -> 0.001 sec )
    vTaskStartScheduler();

    while (true) ;
}


/***************************************************************LED BLINK TEST**************************************************************/

// SemaphoreHandle_t led_semaphore;

// void led_on_task(void *pvParameters) 
// {
//     const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//     const uint16_t Run_tick = 200; // 0.01 sec 
//     const uint16_t Run_time = Run_tick * 10; // ms
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);

//     const TickType_t xPeriod = 500; //500 x 0.01 sec -> 5.0 sec period.
//     TickType_t xLastRequestTime;
//     TickType_t Deadline;

//     xLastRequestTime = xTaskGetTickCount();

//     while (true) 
//     {
//         Deadline = xLastRequestTime + xPeriod;
//         printf("%d + C: %d  Deadline : %d \n", xTaskGetTickCount(), Run_tick ,Deadline);

//         if(xTaskGetTickCount() + Run_tick > Deadline)
//         {
//             printf("LED ON TASK OVERFLOW \n"); 
//             sleep_ms(5000);
//             return;
//         }
//         printf("%d: Task: LED On\n", xTaskGetTickCount());
//         gpio_put(LED_PIN, 1);
//         sleep_ms(Run_time); //LED ON Run-time
//         vTaskDelayUntil(&xLastRequestTime, xPeriod); // Call by reference -> value update.

//         // xSemaphoreGive(led_semaphore);
//         // vTaskDelay(pdMS_TO_TICKS(3000)); //LED OFF Period
//     }
// }




// void led_off_task(void *pvParameters) 
// {
//     const uint LED_PIN = PICO_DEFAULT_LED_PIN;

//     while (true) 
//     {
//         // if (xSemaphoreTake(led_semaphore, portMAX_DELAY) == pdTRUE) 
//         // {
//             printf("Task: LED Off\n");
//             gpio_put(LED_PIN, 0);
//             sleep_ms(5000);
//             // sleep_ms(2000); //LED OFF Run-time
//             vTaskDelay(pdMS_TO_TICKS(7000)); //LED OFF Period
//         // }
//     }
// }

/***************************************************************LED BLINK TEST**************************************************************/
