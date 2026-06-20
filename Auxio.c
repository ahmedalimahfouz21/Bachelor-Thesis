//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <Iolink.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

// Altera
#include "altera_vic_irq.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "Led.h"
#include "iol_hmt.h"


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
enum DirectParamPage {
    VendorParamMirrorOutput = 0x10,
    VendorParamMirrorInput  = 0x11,
    VendorParamPidMode      = 0x12,
    VendorParamTherm        = 0x13,
    VendorParamTemp         = 0x14,
    VendorParamLedFront     = 0x1C,
    VendorParamLedBack      = 0x1D,
    VendorParamLedLeft      = 0x1E,
    VendorParamLedRight     = 0x1F
};

/**
 * Valid values for VendorParamPidMode direct parameter
 */
enum InputDataMode {
    PidDigitalInput,    //!< Update process input data from digital input (push button)
    PidAnalogInput,     //!< Update process input data from analog input (poti)
    PidSawtooth,        //!< Update process input data from a counter
};

uint8_t _ctr;


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
void iolink_start();
void iolink_callback(const IOL_Parameter* param);
void iolink_handleParameterWrite(const IOL_Parameter* param);
void iolink_updateProcessInputData();

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------


void iolink_start() {
    iol_start(&iolink_callback);
}

//------------------------------------------------------------------------------

void iolink_callback(const IOL_Parameter* param) {
    static bool level = false;
    level = !level;

    iol_setSioLevel(level);

    // check for write access to direct parameter page
    if (param) {
        iolink_handleParameterWrite(param);
    }
    else if (iol_stackMode() == STACK_MODE_SIO) {
        // when in SIOActive mode, use digital input to control CQ line
        iol_setSioLevel(level); // button
    }

    // update process data
    iolink_updateProcessInputData();
}

//------------------------------------------------------------------------------

void iolink_handleParameterWrite(const IOL_Parameter* param) {
    // (this is the location to intercept the write access if desired)
    bool commit = true;

    switch (param->address) {
        case VendorParamMirrorOutput:  // mirror data to 0x11:
            iol_parameterWrite(VendorParamMirrorInput, param->value);
            break;

        case VendorParamMirrorInput:  // copy of 0x11 (read only)
            // read-only access => ignore
            commit = false;
            break;

        case VendorParamPidMode:  // process input data selection
            switch (param->value) {
                case PidAnalogInput:
                    // configure ADC
                    break;

                case PidDigitalInput:
                default:
                    break;
            }
            break;

        case VendorParamLedFront:
            printf("Front = %d\n", param->value);
            LedSetColor(tLedPositionFRONT, param->value);
            break;

        case VendorParamLedBack:
            printf("Back = %d\n", param->value);
            LedSetColor(tLedPositionBACK, param->value);
            break;

        case VendorParamLedLeft:
            printf("Left = %d\n", param->value);
            LedSetColor(tLedPositionLEFT, param->value);
            break;

        case VendorParamLedRight:
            printf("Right = %d\n", param->value);
            LedSetColor(tLedPositionRIGHT, param->value);
            break;

        default:
            break;
    };

    if (commit) {
        // commit to stack
        iol_parameterWrite(param->address, param->value);
    }
}

//------------------------------------------------------------------------------

void iolink_updateProcessInputData() {
    static TickType_t last_time = 0;
    TickType_t now = xTaskGetTickCount();

    if (now - last_time >= 10) {
        last_time += 10;
        // the green LED cycles if IO-Link comms are up, red if not
        ++_ctr;
        uint8_t level = ((_ctr >> 3) & 0x0f);
        if (level & 0x8)
            level = ((~level) & 0x7);

        if (iol_masterLost()) {
            // flash the red LED
            iol_setLedLevel(LED_1, LED_LEVEL_OFF);
            iol_setLedLevel(LED_2, (IOL_LedLevel) (level));
        }
        else {
            // flash the green LED
            iol_setLedLevel(LED_1, (IOL_LedLevel) (level));
            iol_setLedLevel(LED_2, LED_LEVEL_OFF);
        };
    }

    // read temperature
    iol_parameterWrite(VendorParamTemp, iol_temperature());

    switch (iol_parameterRead(VendorParamPidMode)) {
        case PidDigitalInput:
            // check digital sensor
            iol_processInputData()->buffer[0] = 0x01;
            iol_processInputData()->isValid = true;
            break;

        case PidAnalogInput:
            ;
            // check analog sensor
            uint16_t sensorValue = 0;
            iol_processInputData()->buffer[0] = sensorValue >> 2;
            iol_processInputData()->isValid = true;
            break;

        case PidSawtooth:
            // copy counter to process input data
            iol_processInputData()->buffer[0] = _ctr;
            iol_processInputData()->isValid = true;
            break;

        default:
            iol_processInputData()->isValid = false;
            break;
    }
}

