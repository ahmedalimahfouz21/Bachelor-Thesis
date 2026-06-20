//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>

// Altera
#include <alt_types.h>
#include <altera_modular_dual_adc.h>
#include <sys/alt_alarm.h>
#include <system.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Project
#include "Vt100.h"
#include "Adc.h"


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
void AdcMain()
{
    Vt100SetInputNonBlocking();
    printf(VT100_HOME VT100_BOLD "ADC test" VT100_NORMAL);
    while(getchar() != VT100_KEY_ESC)
    {
        Vt100SetCursor(3, 1);
        AdcGetAll();
        vTaskDelay(portMS_TO_TICKS(300));
    }
}


void AdcGetAll()
{
    float adc1_voltage[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    float adc2_voltage[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    AdcGetAllRawVoltages(adc1_voltage, adc2_voltage);

    // print
    printf("ADC1.1, 1V2,    %6.2f V\r\n", adc1_voltage[0]);
    printf("ADC1.2, 6V,     %6.2f V\r\n", adc1_voltage[1] * 2.5);
    printf("ADC1.3, 1V8,    %6.2f V\r\n", adc1_voltage[2]);
    printf("ADC1.4, 2V5,    %6.2f V\r\n", adc1_voltage[3] * 2);
    printf("ADC1.5, IV,     %6.2f A\r\n", (adc1_voltage[4] * (825 + 2370) / 2370 - 1.65) / 0.09);
    printf("ADC1.6, VW,     %6.2f V\r\n", adc1_voltage[5] * 102.37 / 2.37);
    printf("ADC1.7, COS1,   %6.2f V\r\n", (adc1_voltage[6] - 1.225) * 4.7 / 9.1);
    printf("ADC1.8, COS2,   %6.2f V\r\n", (adc1_voltage[7] - 1.225) * 4.7 / 9.1);
    printf("ADC1.9, TEMP,   %6.2f V\r\n", adc1_voltage[8] * 1.0);
    printf("ADC2.1, VV,     %6.2f V\r\n", adc2_voltage[0] * 102.37 / 2.37);
    printf("ADC2.2, VX,     %6.2f V\r\n", adc2_voltage[1] * 102.37 / 2.37);
    printf("ADC2.3, VU,     %6.2f V\r\n", adc2_voltage[2] * 102.37 / 2.37);
    printf("ADC2.4, ZK,     %6.2f V\r\n", adc2_voltage[3] * 102.37 / 2.37);
    printf("ADC2.5, IW,     %6.2f A\r\n", (adc2_voltage[4] * (825 + 2370) / 2370 - 1.65) / 0.09);
    printf("ADC2.6, 24V,    %6.2f V\r\n", adc2_voltage[5] * 15.3);
    printf("ADC2.7, SIN1,   %6.2f V\r\n", (adc2_voltage[6] - 1.225) * 4.7 / 9.1);
    printf("ADC2.8, SIN2,   %6.2f V\r\n", (adc2_voltage[7] - 1.225) * 4.7 / 9.1);
    printf("\r\n");
    printf("Press ESC to cancel");
}

void AdcGetMotorValues()
{
    float adc1_voltage[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    float adc2_voltage[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    AdcGetAllRawVoltages(adc1_voltage, adc2_voltage);

    // print
    printf("U: %6.2f V\r\n"           , adc2_voltage[2] * 102.37 / 2.37);
    printf("V: %6.2f V    %6.2f A\r\n", adc2_voltage[0] * 102.37 / 2.37,
                                        (adc1_voltage[4] * (825 + 2370) / 2370 - 1.65) / 0.09);
    printf("W: %6.2f V    %6.2f A\r\n", adc1_voltage[5] * 102.37 / 2.37,
                                        (adc2_voltage[4] * (825 + 2370) / 2370 - 1.65) / 0.09);
    printf("X: %6.2f V\r\n"           , adc2_voltage[1] * 102.37 / 2.37);
}




void AdcGetAllRawVoltages(float *adc1_voltage, float *adc2_voltage)
{
    alt_u32 adc_data[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    alt_u32 adc1_data[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];
    alt_u32 adc2_data[ADC_SAMPLE_STORE_CSR_CSD_LENGTH];

    adc_interrupt_disable(ADC_SAMPLE_STORE_CSR_BASE);
    adc_stop(ADC_SEQUENCER_CSR_BASE);
    adc_recalibrate(ADC_SEQUENCER_CSR_BASE);

    // set to single shot and get one sequence
    adc_set_mode_run_once(ADC_SEQUENCER_CSR_BASE);
    adc_start(ADC_SEQUENCER_CSR_BASE);
    adc_wait_for_interrupt(ADC_SAMPLE_STORE_CSR_BASE);
    adc_clear_interrupt_status(ADC_SAMPLE_STORE_CSR_BASE);
    alt_adc_word_read(ADC_SAMPLE_STORE_CSR_BASE, adc_data, ADC_SAMPLE_STORE_CSR_CSD_LENGTH);

    // separate ADC1 and 2
    for(int k = 0; k < ADC_SAMPLE_STORE_CSR_CSD_LENGTH; k++)
    {
        adc1_data[k] =  adc_data[k]      & 0xFFF;
        adc2_data[k] = (adc_data[k]>>16) & 0xFFF;
        adc1_voltage[k] = adc1_data[k]*2.5/4096;
        adc2_voltage[k] = adc2_data[k]*2.5/4096;
    }
}

