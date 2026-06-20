//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>

// Altera
#include <alt_types.h>
#include <altera_avalon_i2c.h>
#include <system.h>
#include <Temperature.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Project
#include "Vt100.h"

// fmax = 400 kHz

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define TEMPERATURE_I2C_ADDR            (0b1001000) // 7 bit
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
void TemperatureMain() {
    ALT_AVALON_I2C_STATUS_CODE rc;

    const alt_u16 config = 0b1  << 15  // OS      normal operation (cont.)
                         | 0b11 << 13  // R1:0    12 bits, 200ms/conv.
                         | 0b00 << 11  // FT1:0   alarm after 1 fault
                         | 0b0  << 10  // POL     alert pin active low
                         | 0b0  <<  9  // CMP/INT comparator mode
                         | 0b0  <<  8; // SD      sensor active
                                       // lower 8 bits not used

    printf(VT100_HOME VT100_BOLD "Temperature Sensor Test" VT100_NORMAL "\r\n");
    Vt100SetCursor(5, 1);
    printf("Press ESC to cancel");

    // open and configure I2C
    ALT_AVALON_I2C_DEV_t *dev = alt_avalon_i2c_open(TEMP_I2C_NAME);
    if (dev == NULL) {
        Vt100SetCursor(5, 1);
        printf("alt_avalon_i2c_open failed\r\n"
               "Press any key to continue\r\n");
        getchar();
        return;
    }
    alt_avalon_i2c_master_target_set(dev, TEMPERATURE_I2C_ADDR);

    // configure temperatur sensor
    alt_u8 buffer[] = {
            0x01,          // addr of config register
            config >> 8,   // value of config register: upper 8 bits
            config         // ... lower 8 bits
    };

    rc = alt_avalon_i2c_master_tx(dev, buffer, COUNT(buffer),
            ALT_AVALON_I2C_NO_INTERRUPTS);
    if (rc != ALT_AVALON_I2C_SUCCESS) {
        Vt100SetCursor(5, 1);
        printf("alt_avalon_i2c_master_target_set error %d\r\n", (int) rc);
        printf("Press any key to continue\r\n");
        getchar();
        return;
    }

    // read temperature in a loop
    alt_u8 txbuffer[] = { 0x00,             // addr of temperature register
            };
    alt_u8 rxbuffer[2];
    alt_16 temp;

    Vt100SetInputNonBlocking();
    while(getchar() != VT100_KEY_ESC) {
        rc = alt_avalon_i2c_master_tx_rx(dev, txbuffer, COUNT(txbuffer),
                rxbuffer, COUNT(rxbuffer), ALT_AVALON_I2C_NO_INTERRUPTS);
        if (rc != ALT_AVALON_I2C_SUCCESS) {
            Vt100SetCursor(5, 1);
            printf("alt_avalon_i2c_master_target_set error %d\r\n", (int) rc);
            printf("Press any key to continue\r\n");
            Vt100SetInputBlocking();
            getchar();
            return;
        }
        temp = ((alt_16) rxbuffer[0]) << 8 | rxbuffer[1];
        temp /= 16; // ignore 4 '0' LSBs
        Vt100SetCursor(3, 1);
        printf("temperature: raw=%d, %.2f °C           \r", temp, 0.0625 * temp);
        vTaskDelay(300);
    };
}





