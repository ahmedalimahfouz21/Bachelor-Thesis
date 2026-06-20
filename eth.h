//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <alt_types.h>
#include <FreeRtosTest.h>

#include <system.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "queue.h"
#include "message_buffer.h"
#include "stream_buffer.h"

// This file creates some FreeRTOS task which communicate with each other.
// This is useful to test FreeRTOS aware debugging tools.


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------
static SemaphoreHandle_t semaphore;
static SemaphoreHandle_t mutex;
static EventGroupHandle_t events;
static QueueHandle_t queue;
static MessageBufferHandle_t msgbuffer;
static StreamBufferHandle_t streambuffer;
static TimerHandle_t timer;


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
static void freertos_task_mutex(void *context);
static void freertos_task_semaphore(void *context);
static void freertos_task_event(void *context);
static void freertos_task_queue(void *context);
static void freertos_task_msgbuffer(void *context);
static void freertos_task_streambuffer(void *context);
static void freertos_timer_callback(TimerHandle_t th);
static void freertos_push_stack_watermark();


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
void freertos_test_start() {

    // create inter-task communication objects
    mutex = xSemaphoreCreateMutex();
    semaphore = xSemaphoreCreateCounting(10, 0);        // max 10, start 0
    events = xEventGroupCreate();
    timer = xTimerCreate(
            "timer",    // name
            10,         // period
            pdTRUE,     // auto reload
            NULL,       // id output
            freertos_timer_callback
    );
    queue = xQueueCreate(10, 5);                        // 10 items of size 5
    msgbuffer = xMessageBufferCreate(100);              // 100 bytes
    streambuffer = xStreamBufferCreate(100, 10);        // size 100, trigger 10


    vQueueAddToRegistry(mutex, "mutex");
    vQueueAddToRegistry(semaphore, "semaphore");
    vQueueAddToRegistry(queue, "queue");


    // create tasks

    // The NXP plugin seems not to support multiple tasks with the same main function (freertos_task_mutex)
    // The Wittenstein Stateviewer supports it
    xTaskCreate(freertos_task_mutex         , "tskMutex1"   , 1024, NULL, 10, NULL);
    xTaskCreate(freertos_task_mutex         , "tskMutex2"   , 1024, NULL, 11, NULL);
    xTaskCreate(freertos_task_semaphore     , "tsSemphr"    , 1024, NULL, 12, NULL);
    xTaskCreate(freertos_task_event         , "tskEvent"    , 1024, NULL, 13, NULL);
    xTaskCreate(freertos_task_queue         , "tskQueue"    , 1024, NULL, 14, NULL);
    xTaskCreate(freertos_task_msgbuffer     , "tskMsg"      , 1024, NULL, 15, NULL);
    xTaskCreate(freertos_task_streambuffer  , "tskStream"   , 1024, NULL, 16, NULL);


    // start timer
    xTimerStart(timer, portMAX_DELAY);
}

//------------------------------------------------------------------------------


// 2 task instances compete for a mutex
static void freertos_task_mutex(void *context) {
    while(1) {
        if(xSemaphoreTake(mutex, portMAX_DELAY)) {
            vTaskDelay(100);
            xSemaphoreGive(mutex);
        }
        vTaskDelay(10);
        if(xSemaphoreTake(mutex, 1)) {
            vTaskDelay(100);
            xSemaphoreGive(mutex);
        }
    }
}


// This task consumes semaphores
static void freertos_task_semaphore(void *context) {
    while(1) {
        if(xSemaphoreTake(semaphore, portMAX_DELAY));
        if(xSemaphoreTake(semaphore , 0));
        usleep(100);
    }
}


// This task consumes event flags
static void freertos_task_event(void *context) {
    volatile int put_some_bytes_on_the_stack[450] = {0};
    (void)put_some_bytes_on_the_stack; // avoid 'unused' warning
    freertos_push_stack_watermark(5);
    while(1) {
        xEventGroupWaitBits(events, 1, pdTRUE, pdTRUE, portMAX_DELAY);
        xEventGroupWaitBits(events, 1, pdTRUE, pdTRUE, 0);
        usleep(100);
    }
}


// This task consumes messages from a queue
static void freertos_task_queue(void *context) {
    char buf[10];
    while(1) {
        xQueueReceive(queue, buf, portMAX_DELAY);
        xQueueReceive(queue, buf, 0);
        usleep(100);
    }
}


// This task consumes messages from a message buffer
static void freertos_task_msgbuffer(void *context) {
    char buf[10];
    while(1) {
        xMessageBufferReceive(msgbuffer, buf, 10, portMAX_DELAY);
        xMessageBufferReceive(msgbuffer, buf, 10, 0);
        usleep(100);
    }
}


// This task consumes bytes from a buffer
static void freertos_task_streambuffer(void *context) {
    char buf[10];
    while(1) {
        xStreamBufferReceive(streambuffer, buf, 10, portMAX_DELAY);
        xStreamBufferReceive(streambuffer, buf, 10, 0);
        usleep(100);
    }
}



//------------------------------------------------------------------------------


// This task creates the tokens consumed by the tasks above
static void freertos_timer_callback(TimerHandle_t th) {
    xSemaphoreGive(semaphore);
    xEventGroupSetBits(events, 0x01);  // set bit 0
    xQueueSend(queue, "queue", 0);       // timeout = 0
    xMessageBufferSend(msgbuffer, "message", 7, 0);
    xStreamBufferSend(streambuffer, "stream", 6, 0);
}


//------------------------------------------------------------------------------
static void freertos_push_stack_watermark(int i) {
    volatile int some_data[100] = {0};
    (void)some_data;
    if(i > 1) {
        freertos_push_stack_watermark(i-1);
    }
}
