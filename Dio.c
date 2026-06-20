//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

// Altera
#include <alt_types.h>
#include <sys/alt_alarm.h>
#include <io.h>
#include <sys/alt_flash.h>
#include <system.h>

// Project
#include "Vt100.h"
#include "Hyperbus.h"
#include "db_tests.h"


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define KILO                            1024
#define MEGA                            (KILO*KILO)


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
void HyperbusMainRam() {

    printf(VT100_HOME VT100_BOLD "Hyperbus RAM Test" VT100_NORMAL "\r\n\r\n");
    printf("Note: The first 8 MB of RAM are omitted, because that's where this software located\r\n");

    int error = 0;
    int skip    = 8*1024*1024;
    int base    = HYPERRAM_BRIDGE_BASE + skip;
    int amount  = HYPERRAM_BRIDGE_SPAN - skip;



    // Check the data path
    error = error + db_tests_single_data_bit_test(base, 1);

    // Check the byte enable logic
    error = error + db_tests_byte_enable_test(base, 1);

    // Do various other tests and measure the execution time.
    error = error + db_tests_read_write_span_accessibility_test(base, amount, 1);

    printf("\r\n\r\nTotal number of errors detected: %d\r\n\r\n",error);

    printf("\r\n\r\nPress any key to continue");
    getchar();
}



void HyperbusMainFlash()
{
    printf(VT100_HOME VT100_BOLD "Hyperbus Flash Test" VT100_NORMAL "\r\n\r\n");

    int failed = 0;
    int rc;
    uint32_t buffer[1024];
    uint32_t num_words = 1024;


    printf("CFI Test\r\n");
    printf("--------\r\n");
    db_tests_hypermax_check_cfi (HYPERFLASH_BRIDGE_IAVSF_BASE, 1);


    printf("\r\n\r\n\r\n");
    printf("Random Data Write Test\r\n");
    printf("----------------------\r\n");

    // generate random data with system time as seed
    srand(alt_nticks());
    for (int i = 0; i < num_words; i++)
    {
        buffer[i] = rand();
    }

    // copy test data to flash
    alt_flash_fd *fd = alt_flash_open_dev(HYPERFLASH_BRIDGE_IAVSF_NAME);
    if (fd == NULL) {
        printf("Cannot open flash. errno=%d\r\n", errno);
        failed = 1;
    }
    else
    {
        rc = alt_write_flash(fd, 0, (void*) buffer, num_words * 4);
        alt_flash_close_dev(fd);
        if (rc) {
            printf("Error: Flash write returned %d\r\n", rc);
            failed = 1;
        }
        else
        {
            // read back and check (IORD = uncached)
            for (int i = 0; (i < num_words) && !failed; i++) {
              if (IORD_32DIRECT(HYPERFLASH_BRIDGE_IAVSF_BASE, i*4) != buffer[i]) {
                  printf("HyperFlash write test failed at %d\r\n", i);
                  failed = 1;
              }
            }
        }
    }

    if(!failed)
    {
        printf("HyperFlash write test passed\r\n");
    }
    printf("\r\nPress any key to return");
    getchar();
}



#define HYPERFLASH_IO_RD_16( base, word_offs )          IORD_16DIRECT( base, (word_offs*2) )
#define HYPERFLASH_IO_WR_16( base, word_offs, val )     IOWR_16DIRECT( base, (word_offs*2), val )


void HyperbusPrintCfi() {
    uint32_t base = HYPERFLASH_BRIDGE_IAVSF_BASE;

    uint32_t d;


    HYPERFLASH_IO_WR_16( base, 0x555, 0x0071 );     // Issue "Status Register Clear" command
    HYPERFLASH_IO_WR_16( base, 0x555, 0x0070 );     // Issue "Read Status Register" Command
    d = HYPERFLASH_IO_RD_16( base, 0x0 );           // Read the status register value
    printf("Status Register : %04lX\n",d);

    // Do not call any functions (such as printf) that reside in flash while in CFI mode.
    // To be extra-conservative, do not access any HyperRAM memory either.
    //
    HYPERFLASH_IO_WR_16( base, 0x1234, 0x00FF ); // RESET FLASH
    HYPERFLASH_IO_WR_16( base, 0x55,   0x0098 ); // enter CFI Mode.
    for(int i=0; i<=0xFF; i++) {
        d = HYPERFLASH_IO_RD_16(HYPERFLASH_BRIDGE_IAVSF_BASE, i);
        printf("CFI[0x%02X] = 0x%02lX\n", i, d & 0xFF);
    }
    HYPERFLASH_IO_WR_16( base, 0x55, 0x00F0 ); // exit CFI Mode


    HYPERFLASH_IO_WR_16( base, 0x555, 0x0070 );     // Issue "Read Status Register" Command
    d = HYPERFLASH_IO_RD_16( base, 0x0 );               // Read the status register value
}
