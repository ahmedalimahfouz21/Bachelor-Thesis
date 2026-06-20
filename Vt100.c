//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>

// Altera
#include <alt_types.h>
#include <altera_avalon_i2c.h>
#include <altera_avalon_pio_regs.h>
#include <system.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#include "Vt100.h"
#include "Led.h"

// fmax = 400 kHz


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

#define COUNT(arr)                      (sizeof(arr)/sizeof(*arr))


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

void LedMain()
{
    const int sequence[] = { 0, 9, 3, 6,	// R front, right, back, left
            1, 10, 4, 7, // G
            2, 11, 5, 8  // B
            };

    printf(VT100_HOME VT100_BOLD "LED Test" VT100_NORMAL "\r\n\r\n");
    printf("RGB LEDs should be blinking in all 3 colors\r\n\r\n");
    printf("Press ESC to cancel");
    Vt100SetInputNonBlocking();

    while (1)
    {
        for (int i = 0; i < 12; i++)
        {
            IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 1 << sequence[i]);
            vTaskDelay(300);
            if(getchar() == VT100_KEY_ESC)
            {
                LedAllOff();
                return;
            }
        }
    }

}


void LedSetColor(tLedPosition Pos, tLedColor Color)
{
    uint32_t Value = IORD_ALTERA_AVALON_PIO_DATA(LED_BASE);
    Value &= ~(0b111 << Pos);
    Value |= Color << Pos;
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, Value);
}


void LedAllOff()
{
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0);
}


