//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Altera
#include <system.h>
#include <alt_types.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// LwIP
#include "lwip/sockets.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/apps/httpd.h"
#include "Eth.h"

// Project
#include "Temperature.h"
#include "Adc.h"
#include "Crypto.h"
#include "Dio.h"
#include "FreeRtosTest.h"
#include "Hyperbus.h"
#include "Iolink.h"
#include "Irda.h"
#include "Nfc.h"
#include "Led.h"
#include "Motion.h"
#include "Auxio.h"
#include "db_tests.h"
#include "system.h"
#include "Vt100.h"


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------
static void task_init(void *parameters);
static void task_ethernet(void *parameters);
static void task_test(void *pvParameters);
static void execute_test(int number);


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

int main()
{

    // Percepio Tracealyzer
    vTraceEnable(TRC_START);


    xTaskCreate(task_init, "Init",
            configMINIMAL_STACK_SIZE,
            NULL,
            tskIDLE_PRIORITY + 3, (TaskHandle_t*) NULL);

    vTaskStartScheduler();

    // never reached
    asm volatile("break");
    for (;;);
    return 0;
}


//------------------------------------------------------------------------------


static void task_init(void *pvParameters)
{
    // Create some FreeRTOS inter-task communication objects and some task
    // which use them to demonstrate FreeRTOS aware debugging tools.
    //freertos_test_start();

    xTaskCreate(task_test, "Test",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2, (TaskHandle_t*) NULL);

    //iolink_start();

    EthInit();

    LOCK_TCPIP_CORE();
    void *p;
    p = lwiperf_start_tcp_server_default(NULL, NULL);
    //ip_addr_t lwiperf_remote_addr = IPADDR4_INIT_BYTES(192, 168, 1, 2);
    //p = lwiperf_start_tcp_client_default(&lwiperf_remote_addr, NULL, NULL);
    //p = lwiperf_start_tcp_client(&lwiperf_remote_addr, LWIPERF_TCP_PORT_DEFAULT, LWIPERF_DUAL, NULL, NULL);

    UNLOCK_TCPIP_CORE();

    //httpd_init();

    xTaskCreate(task_ethernet, "Socket",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1, (TaskHandle_t*) NULL);

    vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

static void task_ethernet(void *parameters) {

    int rc;
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ip4_addr_t ipaddr;

    IP4_ADDR(&ipaddr, 192,168,1,1);  // concatenates and does htonl()

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = ipaddr.addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char buf[] = "Hello World";

    while(1)
    {
        rc = sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr*)&addr, addrlen);
        vTaskDelay(portMS_TO_TICKS(200));
    }
}


//-----------------------------------------------------------------------------


static void task_test(void *pvParameters)
{
    int digits = 0;
    int number = 0;

    while(1)
    {
        digits = 0;
        number = 0;

        printf(
            VT100_ERASE VT100_HOME
            VT100_BOLD
            "Effectuator 2 Test Suite\r\n"
            VT100_NORMAL
            "\r\n"
            " 1 ADC\r\n"
            " 2 Digital I/O 24 V\r\n"
            " 3 Motion\r\n"
            " 4 HyperRAM\r\n"
            " 5 HyperFlash\r\n"
            " 6 NFC\r\n"
            " 7 IRDA\r\n"
            " 8 Temperature Sensor\r\n"
            " 9 RGB LEDs\r\n"
            "10 Auxiliary IOs (tested as Outputs)\r\n"
            "\r\n"
            "Other tests:\r\n"
            "Keys      : press Key 1 or 2 and check if lit\r\n"
            "Reset Key : press and hold key1. The system should reset after approx. 17 seconds.\r\n"
            "ETH1      : IP=192.168.1.20, iperf server running + periodic UDP packets to 192.168.1.1:8000\r\n"
            "            Back RGB LED is used for status: R=link, G=act; both are or'd with values from test software.\r\n"
            "\r\n"
            VT100_BOLD
            "Input number: "
        );

        // Let the user input the test number
        Vt100SetInputBlocking();
        while(1)
        {
            int c = getchar();

            // always accept ESC --> restart
            if(c == VT100_KEY_ESC)
            {
                break;
            }

            // accept digits if we do not already have 2 digits
            if(('0' <= c) && (c <= '9') && (digits < 2))
            {
                number = number * 10 + c - '0';
                digits++;
                putchar(c);
            }

            // accept return if the currently entered number is a valid test
            if(((c == 10) || (c == 13)) && (1 <= number) && (number <= 99))
            {
                printf(VT100_ERASE VT100_HOME VT100_NORMAL);
                execute_test(number);
                break;
            }

            // accept backspace if the number is not empty
            if((digits > 0) && (c == VT100_KEY_BACKSPACE))
            {
                number /= 10;
                digits--;
                putchar(VT100_KEY_BACKSPACE);
                putchar(' ');
                putchar(VT100_KEY_BACKSPACE);
            }
        }
    }
}


//------------------------------------------------------------------------------



static void execute_test(int number)
{
    switch(number)
    {
        case 1:
            AdcMain();
            break;

        case 2:
            DioMain();
            break;

        case 3:
            MotionMain();
            break;

        case 4:
            HyperbusMainRam();
            break;

        case 5:
            HyperbusMainFlash();
            break;

        case 6:
            NfcMain();
            break;

        case 7:
            IrdaMain();
            break;

        case 8:
            TemperatureMain();
            break;

        case 9:
            LedMain();
            break;

        case 10:
            AuxioMain();
            break;

        default:
            break;
    }
}


//------------------------------------------------------------------------------



void _general_exception_handler(unsigned long ulCause, unsigned long ulStatus) {
    /* This overrides the definition provided by the kernel.  Other exceptions
     should be handled here. */
    for (;;) {
        asm( "break" );
    }
}




//------------------------------------------------------------------------------
// FreeRTOS Hooks
//------------------------------------------------------------------------------

void vApplicationIdleHook()
{
    // no WFI instruction in NIOS2?
}

void vApplicationStackOverflowHook()
{
    //Look at pxCurrentTCB to see which task overflowed its stack.
    asm( "break" );
}

void vAssertCalled(char *file, int line)
{
    printf("FreeRTOS assert failed in %s at line %d\n", file, line);
}

#if configSUPPORT_STATIC_ALLOCATION==1
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t IdleTaskTCBBuffer;
    static StackType_t IdleTaskStackBuffer[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &IdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = IdleTaskStackBuffer;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
#endif

#if (configSUPPORT_STATIC_ALLOCATION==1) && (configUSE_TIMERS==1)
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
        StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    static StaticTask_t TimerTaskTCBBuffer;
    static StackType_t TimerTaskStackBuffer[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &TimerTaskTCBBuffer;
    *ppxTimerTaskStackBuffer = TimerTaskStackBuffer;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif
