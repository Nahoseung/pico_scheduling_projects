#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "pico/stdlib.h"

#define sync_R 100  // vTaskDelay(pdMS_TO_TICKS(sync_R));  delay 10tick

void vTask0(void* pvParameters);
void vTask1(void* pvParameters);
void vTask2(void* pvParameters);
void vTask3(void* pvParameters);
void vTask4(void* pvParameters);
void vTask5(void* pvParameters);

task_info task_list [MAX_NUM_TASKS] =
{

        /* Code /  Name  / Run / Period / Core / Priority / Subnum / my_ptr / splitted_ptr / Dependency / Heavy */

        {vTask0, "TASK 0",  20, 100,  0, 5,1, NULL, NULL, false,false}, // TASK 0
        {vTask1, "TASK 1",  80, 200,  0, 4,1, NULL, NULL, false,false}, // TASK 1
        {vTask2, "TASK 2",  300, 400,  0, 3,1, NULL, NULL, false,false},  // TASK 2
        {vTask3, "TASK 3",   20,  400,  0, 2,1, NULL, NULL,  false,false},
        {vTask4, "TASK 4",   20,  400,  0, 1,1, NULL, NULL,  false,false},
        {vTask5, "Extra T",   0,   0,  0, 1,1, NULL, NULL,  false,false}
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

    core_manager.Utilization_Bound = Utilization_Bound;
    pre_assigned_core.Utilization_Bound = Utilization_Bound;

    // * UQ, PQ_pre stack들을 초기화
    init_core_stack(&pre_assigned_core);
    init_task_stack(&Normal_task);
    



    /*
     * ******************** SPA2  ************************
     */

    for(int i=0; i < NUM_OF_TASK; i++)
    {
        task_info* T= task_manager.list[i];
        if(T->Heavy&&simple_test(T,i,&core_manager,&task_manager))
        {
            core_info* C = Pop_core(&core_manager);
            Assign_task(T,C);
            C->pre_assigned = true;
            Push_core(C,&pre_assigned_core);
        }
        else
        {
            Push_task(T,&Normal_task);
        }
    }

    while(!T_is_empty(&Normal_task))
    {
        core_info* min_C = get_min_core(&core_manager);
        task_info* T = Pop_task(&Normal_task);

        if(min_C->Utilization >= Utilization_Bound)
        {
            min_C = Pop_core(&pre_assigned_core);
            if(min_C->Utilization >= Utilization_Bound)
            {
                break;
            }
        }

        if(min_C->Utilization + T->Utilization <= Utilization_Bound)
        {
            // * !
            Assign_task(T,min_C);
            
            if(min_C->pre_assigned)
            {
                Push_core(min_C,&pre_assigned_core);
            }
        }
        else
        {
            // * !
            Task_split(T,min_C,&core_manager,&Normal_task);
            Assign_task(T,min_C);
        }
    }
    printf("Core 0 Utilization : %.3f, Core 1 Utilization : %.3f \n" , core_manager.list[0]->Utilization, core_manager.list[1]->Utilization);

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


void vTask3(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 3;
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

void vTask4(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 4;
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

void vTask5(void *pvParameters) 
{ 
    /****** Tick (10ms) 기준 Runtime, Period 부여 ******/

    /*******************************/
    static const uint8_t task_num= 5;
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