//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>


// Altera
#include <alt_types.h>
#include <altera_avalon_pio_regs.h>
#include <io.h>
#include <system.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#include "Vt100.h"
#include "Adc.h"
#include "Motion.h"

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
static void MotionSetPhase(int phase, int value);
static void MotionSetEnabled(bool enabled);
static void MotionSetBlockCommutation(bool block_commutation);
static void MotionSetCommutationMode(bool use_3phases);
static void MotionSetSwitchUW(bool switch_u_w);


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

void MotionMain() {

    int index;
    int change;
    bool upper_case;

    int pwm[4] = {0, 0, 0, 0};
    bool enable = false;
    bool block_commutation = false;
    bool use_3phases = false;
    bool switch_u_w = false;

    // Show initial screen
    printf(
         VT100_HOME VT100_BOLD "BLDC Motor test" VT100_NORMAL "\r\n"
         "\r\n"
         "PWM U            :                       q = up, a = down\r\n"
         "PWM V            :                       w = up, s = down\r\n"
         "PWM W            :                       e = up, d = down\r\n"
         "PWM X            :                       r = up, f = down\r\n"
         "\r\n"
         "Enable           :                       space = toggle\r\n"
         "Mode             :                       m = toggle\r\n"
         "Commutation Mode :                       c = toggle\r\n"
         "Switch U/W       :                       x = toggle (for motor test on FCC connector)\r\n");

     Vt100SetCursor(25, 1);
     printf(
         "Hints:  - Use lower case letters to change PWM in +/-1 steps or upper case for +/-10 steps\r\n"
         "        - Use PWM U value in block block commutation mode (V and W have no effect in this mode) \r\n"
         "        - Press ESC to return\r\n"
     );

    // Set initial values
    MotionSetEnabled(enable);
    for(int i = 0; i < 4; i++)
    {
        MotionSetPhase(i, pwm[i]);
    }
    MotionSetBlockCommutation(block_commutation);
    MotionSetCommutationMode(use_3phases);
    MotionSetSwitchUW(switch_u_w);

    // main loop
    Vt100SetInputNonBlocking();
    while (1)
    {

        int c = getchar();

        // convert input to lower case, but remember if it was upper case
        if(('A' <= c) && (c <= 'Z'))
        {
            c -= 'A' - 'a';
            upper_case = true;
        }
        else
        {
            upper_case = false;
        }


        // on ESC return
        if (c == VT100_KEY_ESC)
        {
            MotionSetEnabled(false);
            return;
        }

        // check if input is up/down for any motor phase
        switch(c)
        {
            case 'q':
                index = 0;
                change = 1;
                break;
            case 'a':
                index = 0;
                change = -1;
                break;
            case 'w':
                index = 1;
                change = 1;
                break;
            case 's':
                index = 1;
                change = -1;
                break;
            case 'e':
                index = 2;
                change = 1;
                break;
            case 'd':
                index = 2;
                change = -1;
                break;
            case 'r':
                index = 3;
                change = 1;
                break;
            case 'f':
                index = 3;
                change = -1;
                break;
            default:
                index = -1;
        }

        // if input was up/down for any motor phase: calculate and set new value
        if(index >= 0)
        {
            if(upper_case)
            {
                change *= 10;
            }
            pwm[index] += change;
            if(pwm[index] > 90)
            {
                pwm[index] = 90;
            }
            else if(pwm[index] < -90)
            {
                pwm[index] = -90;
            }
            MotionSetPhase(index, pwm[index]);
        }

        else if(c == ' ')
        {
            enable = !enable;
            MotionSetEnabled(enable);
        }

        else if(c == 'm')
        {
            block_commutation = !block_commutation;
            MotionSetBlockCommutation(block_commutation);
        }

        else if(c == 'c')
        {
            use_3phases = !use_3phases;
            MotionSetCommutationMode(use_3phases);
        }

        else if(c == 'x')
        {
            switch_u_w = !switch_u_w;
            MotionSetSwitchUW(switch_u_w);
        }

        else
        {
            vTaskDelay(portMS_TO_TICKS(300));
        }
        Vt100SetCursor(13, 1);
        MotionShowInputs();
        printf("\r\n\r\n");
        AdcGetMotorValues();
    }
}


void MotionShowInputs()
{

    uint32_t din = IORD_ALTERA_AVALON_PIO_DATA(DIN_BASE);
    uint32_t hall = IORD(BLDC_CONTROLLER_BASE, 1);

    printf("DIN_FAULT_IV = %c\r\n", (din & 1) ? 'x' : '.');
    printf("DIN_FAULT_IW = %c\r\n", (din & 2) ? 'x' : '.');
    printf("DIN_OVCUR    = %c\r\n", (din & 4) ? 'x' : '.');
    printf("Hall Sensors = %c %c %c\r\n",
           (hall & 4) ? 'x' : '.',
           (hall & 2) ? 'x' : '.',
           (hall & 1) ? 'x' : '.');
}


static void MotionSetPhase(int phase, int value)
{
    int val = 128 + 128.0 / 100.0 * value;
    if (val > 254)
    {
        val = 254;
    }
    Vt100SetCursor(3 + phase, 20);
    printf("%4d %%", value);
    IOWR(BLDC_CONTROLLER_BASE, 4 + phase, val);
}


static void MotionSetEnabled(bool enabled)
{
    uint32_t value = IORD(BLDC_CONTROLLER_BASE, 0);
    Vt100SetCursor(8, 20);
    if(enabled)
    {
        printf("enabled ");
        value |= 1ul;
    }
    else
    {
        printf("disabled");
        value &= ~1ul;
    }
    IOWR(BLDC_CONTROLLER_BASE, 0, value);
}

static void MotionSetBlockCommutation(bool block_commutation)
{
    uint32_t value = IORD(BLDC_CONTROLLER_BASE, 0);
    Vt100SetCursor(9, 20);
    if(block_commutation)
    {
        printf("block commutation");
        value |= 2ul;
    }
    else
    {
        printf("manual PWM       ");
        value &= ~2ul;
    }
    IOWR(BLDC_CONTROLLER_BASE, 0, value);
}

static void MotionSetCommutationMode(bool use_3phases)
{
    uint32_t value = IORD(BLDC_CONTROLLER_BASE, 0);
    Vt100SetCursor(10, 20);
    if(use_3phases)
    {
        printf("3 Phases");
        value |= 4ul;
    }
    else
    {
        printf("2 Phases");
        value &= ~4ul;
    }
    IOWR(BLDC_CONTROLLER_BASE, 0, value);
}


static void MotionSetSwitchUW(bool switch_u_w)
{
    uint32_t value = IORD(BLDC_CONTROLLER_BASE, 0);
    Vt100SetCursor(11, 20);
    if(switch_u_w)
    {
        printf("switched    ");
        value |= 8ul;
    }
    else
    {
        printf("not switched");
        value &= ~8ul;
    }
    IOWR(BLDC_CONTROLLER_BASE, 0, value);
}

