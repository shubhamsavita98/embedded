#ifndef __STM32L476G_ADC_H
#define __STM32L476G_ADC_H

#include "stm32l476xx.h"


extern volatile uint32_t adc_result; //Declaration of global variable to store sampled ADC data 
extern  uint32_t temperature_C;

// Modular function to wake up ADC1 from the deep-power-down mode 
void ADC1_Wakeup (void);

// Modular function to initialize ADC external input channels
// In this sample, PA1 (ADC12_IN6) is used
void ADC_Pin_Init(void);

// Modular function to configure ADC common registers
void ADC_Common_Configuration(void);

// Modular function to initialize ADC
void ADC_Init(void);



#endif /* __STM32L476G_ADC_H */
