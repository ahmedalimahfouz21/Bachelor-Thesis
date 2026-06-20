//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

// Altera
#include <altera_avalon_pio_regs.h>


// Project
#include "system.h"
#include "Vt100.h"
#include "Auxio.h"


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
typedef struct
{
    char *name;
    int bit_number;
    char toggle_key;
} tAuxioPin;


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------
static const tAuxioPin AuxioPins[] =
{
    { "NC-Z210" ,  0, 'q'},
    { "NC-Z211" ,  1, 'w'},
    { "NC-Z251" ,  2, 'e'},
    { "NC-Z252" ,  3, 'r'},
    { "NC-Z253" ,  4, 't'},
    { "B7-B13"  ,  5, 'z'},
    { "B7-E10"  ,  6, 'u'},
    { "B7-F12"  ,  7, 'i'},
    { "B7-C13"  ,  8, 'o'},
    { "B7-D12"  ,  9, 'p'},
    { "B7-A14"  , 10, 'a'},
    { "B7-A13"  , 11, 's'},
    { "B7-F10"  , 12, 'd'},
    { "B7-F11"  , 13, 'f'},
    { "B7-C12"  , 14, 'g'},
    { "B7-E11"  , 15, 'h'},
    { "NC-F306" , 16, 'j'}
};

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
void AuxioMain()
{
    uint32_t value = 0;
    IOWR_ALTERA_AVALON_PIO_DATA(PIO_AUX_BASE, value);

    // Display title and initial data
    printf(VT100_HOME VT100_BOLD "Auxiliary IO Test" VT100_NORMAL "\r\n\r\n");

    Vt100SetCursor(3, 1);
    printf("IO Pin   Value   Toggle key\r\n");
    printf("---------------------------\r\n");
    for(int i = 0; i < sizeof(AuxioPins) / sizeof(tAuxioPin); i++)
    {
        printf("%-8s %c       %c\r\n",
                AuxioPins[i].name,
                (1 << AuxioPins[i].bit_number) & value ? 'x' : '.',
                AuxioPins[i].toggle_key);
    }
    printf("\r\n\r\nx = On, . = Off. \r\nPress ESC to return.\r\n");

    while(1)
    {
        char c = getchar();

        if(c == VT100_KEY_ESC)
        {
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_AUX_BASE, 0);
            return;
        }

        for(int i = 0; i < sizeof(AuxioPins) / sizeof(tAuxioPin); i++)
        {
            if(tolower(c) == AuxioPins[i].toggle_key)
            {
                uint32_t mask = 1 << AuxioPins[i].bit_number;
                value ^= mask;
                Vt100SetCursor(5 + i, 10);
                putchar(value & mask ? 'x' : '.');
                IOWR_ALTERA_AVALON_PIO_DATA(PIO_AUX_BASE, value);
            }
        }

    }
}













