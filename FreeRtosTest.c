//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

// Altera
#include <alt_types.h>
#include <altera_avalon_pio_regs.h>
#include <system.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Project
#include "Vt100.h"
#include "Dio.h"

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

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
void DioMain() {

    bool output_enable = false;
    int input;
    int i = 0;

    // Show static texts
    printf(VT100_HOME VT100_BOLD "Digital I/O Test" VT100_NORMAL);
    Vt100SetCursor(3, 21);
    printf("1 2 3 4 5 6 7 8");
    Vt100SetCursor(5, 1);
    printf("Outputs");
    Vt100SetCursor(7, 1);
    printf("Inputs");
    Vt100SetCursor(12,1);
    printf("Outputs (open drain) : x = driven low   . = High-Z\r\n");
    printf("Inputs               : x = high,        . = low\r\n\r\n");
    printf("Press e to toggle output enable. " VT100_BOLD "Do not use if any IO is driven externally.\r\n" VT100_NORMAL);
    printf("Press ESC to cancel.");

    // Loop
    Vt100SetInputNonBlocking();
    while(1)
    {

        // Set current output and display output state, if the outputs are enabled, then go to the next output
        if(output_enable)
        {
            i = (i + 1) % 8;
            IOWR_ALTERA_AVALON_PIO_DATA(DIO_OE_BASE, 0xFF);
            IOWR_ALTERA_AVALON_PIO_DATA(DIO_OUT_BASE, 1 << i);
            Vt100SetCursor(5, 9);
            printf("enabled     ");
            for(int j = 0; j < 8; j++) {
                printf(i == j ? "x " : ". ");
            }
        }

        // Disable all outputs if set to disable
        else
        {
            IOWR_ALTERA_AVALON_PIO_DATA(DIO_OE_BASE, 0);
            Vt100SetCursor(5, 9);
            printf("all High-Z                            ");
            i = 7;

        }
        vTaskDelay(1);

        // Read all inputs and display their values
        input = IORD_ALTERA_AVALON_PIO_DATA(DIO_IN_BASE);
        Vt100SetCursor(7, 21);
        for(int j = 0; j < 8; j++)
        {
            printf(input & (1 << j) ? "x " : ". ");
        }

        // Wait some time
        vTaskDelay(portMS_TO_TICKS(300));

        // React on keystrokes: ESC to cancel, 'e' to toggle output enable
        int c = getchar();
        if(c == VT100_KEY_ESC)
        {
            break;
        }
        else if(c == 'e')
        {
            output_enable = !output_enable;
        }
    }

    // Before returning, make sure the outputs are disabled to avoid possible short circuits
    IOWR_ALTERA_AVALON_PIO_DATA(DIO_OE_BASE, 0);
}
