//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>

// Altera
#include <alt_types.h>
#include <altera_avalon_i2c.h>
#include <altera_avalon_pio_regs.h>


// Project
#include "system.h"
#include "Vt100.h"
#include "Led.h"
#include "Nfc.h"

// fmax = 400 kHz

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define NFC_I2C_ADDR                    (0b1010101) // 7 bit
#define I2C_FREQ                        400000
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
void NfcMain()
{
    ALT_AVALON_I2C_STATUS_CODE rc;
    alt_u8 txbuffer[1] = { 0 }; // start address of serial number
    alt_u8 rxbuffer[7];       // serial number is 7 bytes long

    printf(VT100_HOME VT100_BOLD "NFC Test" VT100_NORMAL "\r\n\r\n");

    // open I2C
    ALT_AVALON_I2C_DEV_t *dev = alt_avalon_i2c_open(NFC_I2C_NAME);
    if (dev == NULL) {
        printf("alt_avalon_i2c_open failed\r\n");
        printf("Press any key to continue");
        getchar();
        return;
    }

    // configure
    /*
     ALT_AVALON_I2C_MASTER_CONFIG_t config = {
     ALT_AVALON_I2C_ADDR_MODE_7_BIT,
     ALT_AVALON_I2C_SPEED_FAST,
     ALT_CPU_CPU_FREQ/(I2C_FREQ*2), // SCL high count
     ALT_CPU_CPU_FREQ/(I2C_FREQ*2), // SCL low count
     ALT_CPU_CPU_FREQ/(I2C_FREQ*2), // SDA hold count
     };
     alt_avalon_i2c_master_config_set(dev, &config);
     */
    alt_avalon_i2c_master_target_set(dev, NFC_I2C_ADDR);

    // read serial number
    rc = alt_avalon_i2c_master_tx_rx(dev, txbuffer, COUNT(txbuffer), rxbuffer,
            COUNT(rxbuffer), ALT_AVALON_I2C_NO_INTERRUPTS);

    if (rc != ALT_AVALON_I2C_SUCCESS)
    {
        printf("alt_avalon_i2c_master_tx_rx returned error code %ld\r\n", rc);
        printf("Press any key to continue");
        getchar();
        return;
    }

    printf("serial number: ");
    for (int i = 0; i < COUNT(rxbuffer); i++)
    {
        printf("%02X ", rxbuffer[i]);
    }

    Vt100SetCursor(7,1);
    printf("Press ESC to cancel");

    Vt100SetInputNonBlocking();
    while(getchar() != VT100_KEY_ESC)
    {
        Vt100SetCursor(5,1);
        if(IORD_ALTERA_AVALON_PIO_DATA(NFC_FD_BASE))
        {
            printf("No NFC field       ");
            LedSetColor(tLedPositionFRONT, tLedColorRED);
            LedSetColor(tLedPositionBACK,  tLedColorRED);
            LedSetColor(tLedPositionLEFT,  tLedColorRED);
            LedSetColor(tLedPositionRIGHT, tLedColorRED);
        }
        else
        {
            printf("NFC field detected ");
            LedSetColor(tLedPositionFRONT, tLedColorGREEN);
            LedSetColor(tLedPositionBACK,  tLedColorGREEN);
            LedSetColor(tLedPositionLEFT,  tLedColorGREEN);
            LedSetColor(tLedPositionRIGHT, tLedColorGREEN);
        }
        vTaskDelay(portMS_TO_TICKS(300));
    }

    LedAllOff();
}












