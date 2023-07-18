 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"
#include <math.h>
#include <stdio.h>

#define SINE_PERIOD_STEPS               (100UL)                     /* number of samples for a sine wave period */
#define SINE_AMPLITUDE                  (511UL)                     /* LSB steps */
#define DC_OFFSET                       (512UL)                     /* LSB steps */

#define MAX_PERIOD_COUNT                (65024UL)                   /* value from TMR1 register */
#define MAX_ADC_RESULT                  (2541UL)                    /* POT 3 VREF = 2V, VDD = 3.3V */

#define ONE_QUARTER_OF_MAX_ADC_RESULT   (MAX_ADC_RESULT / 4)
#define FIRST_RANGE_INITIAL_VALUE       (1)
#define SECOND_RANGE_INITIAL_VALUE      (MAX_PERIOD_COUNT - 3 * ONE_QUARTER_OF_MAX_ADC_RESULT)
#define MULTIPLICATION_FACTOR           (SECOND_RANGE_INITIAL_VALUE * 100 / ONE_QUARTER_OF_MAX_ADC_RESULT)

/* Buffer to store the sine wave samples */
uint16_t sineWave[SINE_PERIOD_STEPS];
uint16_t adcResult = 0;
uint8_t sineIndex = 0;
uint16_t newPeriodCount = 0;

static void sineWaveInit(void)
{
    uint8_t i;
    for(i = 0; i < SINE_PERIOD_STEPS; i++)
    {
        sineWave[i] = DC_OFFSET + (uint16_t)(SINE_AMPLITUDE * sin(2 * M_PI * i / SINE_PERIOD_STEPS));
    }
}

void TMR1_Handler(void)
{
    DAC1_SetOutput(sineWave[sineIndex++]);
    if(sineIndex == SINE_PERIOD_STEPS)
    {
        sineIndex = 0;
    }
}

void ADC_Handler(void)
{
    adcResult = ADC_GetConversionResult();
    if(adcResult == 0)
    {
        newPeriodCount = FIRST_RANGE_INITIAL_VALUE;
    }    
    else if(adcResult <= ONE_QUARTER_OF_MAX_ADC_RESULT)
    {
        newPeriodCount = (uint16_t)(((uint32_t)(MULTIPLICATION_FACTOR * adcResult)) / 100);
    }
    else if(adcResult > ONE_QUARTER_OF_MAX_ADC_RESULT)
    {
        newPeriodCount = SECOND_RANGE_INITIAL_VALUE + (adcResult - ONE_QUARTER_OF_MAX_ADC_RESULT);
    }
    TMR1_PeriodCountSet(newPeriodCount);
}

/*
    Main application
*/

int main(void)
{ 
    SYSTEM_Initialize();
    
    TMR1_OverflowCallbackRegister(TMR1_Handler);
    ADC_SetADIInterruptHandler(ADC_Handler);
    
    sineWaveInit();
    
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts 
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts 
    // Use the following macros to: 

    //Enable the Global High Interrupts 
    INTERRUPT_GlobalInterruptHighEnable(); 

    // Disable the Global High Interrupts 
    //INTERRUPT_GlobalInterruptHighDisable(); 

    // Enable the Global Low Interrupts 
    INTERRUPT_GlobalInterruptLowEnable(); 

    // Disable the Global Low Interrupts 
    //INTERRUPT_GlobalInterruptLowDisable(); 
    
    while(1)
    {
        switch (APM_StatusGet())
        {
            case 0:
                APMStatus_SetLow();
                ADCAStatus_SetLow();
                DAC1Status_SetLow();
                break;   
            case _APMSTATUSL_ADCA_MASK:
                APMStatus_SetHigh();
                ADCAStatus_SetHigh();
                break;       
            case ((uint16_t)_APMSTATUSH_DAC1_MASK << 8 | _APMSTATUSL_ADCA_MASK):
                DAC1Status_SetHigh();
                break;
        }
    }
}