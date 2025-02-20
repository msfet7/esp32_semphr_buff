/** 
 @author Mateusz Szpot
 @brief Code demonstrating usage of semaphores and mutexes
 @brief It is an implementation of a simple buffer protected by semaphores
*/ 

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"


#define UP_NUM_RANGE 10                            // the highest number to send
#define BUFF_SIZE 10                               // size of a buffer

// Semaphores
SemaphoreHandle_t xEmiterHeadSemphr = NULL;       // semphr indicating, that variable from emitter task is ready
SemaphoreHandle_t xBufferMutex = NULL;            // mutex to handle buffer
SemaphoreHandle_t xEmptyCountSemphr = NULL;
SemaphoreHandle_t xFullCountSemphr = NULL;        // semphrs responsible for counting enpty or filled fields of a buffer

// Buffer
static uint8_t buffer[BUFF_SIZE] = {0};
static uint8_t writeIndex = 0;                    // used for writing items to buffer
static uint8_t readIndex = 0;                     // used for reading items from buffer

// Variables
static uint8_t emitterHead = 0;

//Tasks

// emiter task
void xEmiterTask(void* arg){
    uint8_t toSend = 1;
    while(1){
        //ESP_LOGI("EMITTER", "HELLO");
        emitterHead = toSend;
        toSend = (toSend  % UP_NUM_RANGE) + 1;
        xSemaphoreGive(xEmiterHeadSemphr);
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}

// buffer handler task
void xBufferHandler(void* arg){
    while(1){
        if(xSemaphoreTake(xEmiterHeadSemphr, (TickType_t) 50) == pdTRUE){
            xSemaphoreTake(xEmptyCountSemphr, portMAX_DELAY);
            xSemaphoreTake(xBufferMutex, portMAX_DELAY);
            // critical section
            buffer[writeIndex] = emitterHead;
            writeIndex = (writeIndex + 1) % BUFF_SIZE;
            xSemaphoreGive(xBufferMutex);
            xSemaphoreGive(xFullCountSemphr);
        }
    }
}

// executive task
void xExecutiveTask(void* arg){
    float mean = 0;
    while(1){
        for(uint8_t cnt = 0; cnt < BUFF_SIZE; cnt++){
            xSemaphoreTake(xFullCountSemphr, portMAX_DELAY);
            xSemaphoreTake(xBufferMutex, portMAX_DELAY);
            // critical section
            mean += buffer[readIndex];
            readIndex = (readIndex + 1) % BUFF_SIZE;
            xSemaphoreGive(xBufferMutex);
            xSemaphoreGive(xEmptyCountSemphr);
        }

        mean /= BUFF_SIZE;
        ESP_LOGI("EXEC: ", "The mean value of recived values from the buffer is: %.2f", mean);
        mean = 0;
    }
}

void app_main(void)
{
    // semaphores creation
    xEmiterHeadSemphr = xSemaphoreCreateBinary();
    if(xEmiterHeadSemphr != NULL) ESP_LOGI("SEMPHR: ", "Emiter head semaphore created sucesfully!!!");
    else ESP_LOGE("SEMPHR: ", "Emiter head semaphore created unsucesfully!!!");

    xBufferMutex = xSemaphoreCreateMutex();
    if(xBufferMutex != NULL) ESP_LOGI("SEMPHR: ", "Buffer mutex created sucesfully!!!");
    else ESP_LOGE("SEMPHR: ", "Buffer mutex created unsucesfully!!!");

    xEmptyCountSemphr = xSemaphoreCreateCounting(BUFF_SIZE, BUFF_SIZE);
    xFullCountSemphr = xSemaphoreCreateCounting(BUFF_SIZE, 0);
    if(xEmptyCountSemphr != NULL && xFullCountSemphr != NULL) ESP_LOGI("SEMPHR: ", "Counting semaphores created sucesfully!!!");
    else ESP_LOGE("SEMPHR: ", "Counting semaphores created unsucesfully!!!");


    // tasks creation
    xTaskCreate(xExecutiveTask, "EXECUTIVE", 2048, NULL, 2, NULL);
    xTaskCreate(xBufferHandler, "BUFFER", 2048, NULL, 2, NULL);
    xTaskCreate(xEmiterTask, "EMITER", 2048, NULL, 2, NULL);
}