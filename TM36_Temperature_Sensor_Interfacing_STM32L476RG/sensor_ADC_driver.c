#include "sensor_ADC_driver.h"
#include "stm32l476xx.h"
#include <stdint.h>

volatile uint32_t adc_result = 0; //Definition of global variable 'adc_result' declared in "ADC.h"

//-------------------------------------------------------------------------------------------
// ADC1 Wakeup
// By default, the ADC modules are in deep-power-down mode where their power supply is internally switched off
// to reduce the leakage currents.
//-------------------------------------------------------------------------------------------
void ADC1_Wakeup (void) {
	
	int wait_time;
	
	// To start ADCx operations, the following sequence should be applied through ADCx_CR register:
	// 1. Exit deep power down mode 
	// 		DEEPPWD = 0: ADC not in deep-power down
	// 		DEEPPWD = 1: ADC in deep-power-down (default reset state)
	ADC1->CR &= ~ADC_CR_DEEPPWD;
	
	// 2. Enable the ADC internal voltage regulator
	//    Before performing any operation such as launching a calibration or enabling the ADC, the ADC
	//    voltage regulator must first be enabled and the software must wait for the regulator start-up time (T_ADCVREG_STUP) .
	ADC1->CR |= ADC_CR_ADVREGEN;	
	
	// 3. Wait for ADC voltage regulator start-up time (T_ADCVREG_STUP) 
	//    T_ADCVREG_STUP for STM32L476x MCUs is 20 us
	//    The default processor clock is 4MHz provided by MSI. 
  //    Note: The following implementation for generating a 20 us delay is not optimally precise.
  //    It is, however, adequate for satisfying the minimum required start-up time.
	wait_time = 20 * (4000000 / 1000000);
	while(wait_time != 0) {
		wait_time--;
	} 
}
//-------------------------------------------------------------------------------------------
// 	Configuration of ADC Common Registers
//-------------------------------------------------------------------------------------------	
void ADC_Common_Configuration(){
	
	// 1. Select ADC input clock through ADC_CCR register, field CKMODE[1:0]
	//	  The ADC input clock can be a specific clock source (asynchronous clock mode),
	//    or derived from the AHB clock of the ADC bus interface (synchronous clock mode), divided by a 
	//    programmable factor (1, 2, or 4).
	// 
	//   ADC_CCR register, field CKMODE[1:0] values:
	//   -00: CK_ADCx (x=123) (Asynchronous clock mode),
	//   -01: HCLK/1 (Synchronous clock mode)
	//   -10: HCLK/2 (Synchronous clock mode)
	//   -11: HCLK/4 (Synchronous clock mode)	 
	//   In this sample, HCLK/1 (01) is selected, meaning that the ADC input clock is 
	//	 synchronous to AHB clock, equivalent to the default processor clock 4MHz
	ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE;   //clear both bits first
	ADC123_COMMON->CCR |=  ADC_CCR_CKMODE_0; // set CKMODE[1:0] to ‘01’
	//ADC123_COMMON->CCR |=  ADC_CCR_CKMODE_1 | ADC_CCR_CKMODE_0; // set CKMODE[1:0] to ‘11’
	
	// 2. Configure the ADC clock prescaler through ADC_CCR register, field PRESC[3:0]
	//    ADC_CCR register, field PRESC[3:0] values:
	//    -0000: input ADC clock not divided
  //    For information about other configuration values, refer to the reference manual	
	ADC123_COMMON->CCR &= ~ADC_CCR_PRESC; 
	ADC123_COMMON->CCR |= ADC_CCR_PRESC_3 | ADC_CCR_PRESC_1 | ADC_CCR_PRESC_0 ; //divided by 256
	
	// 3. Configure the ADC dual mode through ADC_CCR register, field DUAL[4:0]
	//		ADC_CCR register, field DUAL[4:0] values:
	//    -00000: Independent mode
  //    For information about other configuration values, refer to the reference manual	
	ADC123_COMMON->CCR &= ~ADC_CCR_DUAL;
	
	//ADC123_COMMON->CCR &= ~ADC_CCR_DELAY;
	//ADC123_COMMON->CCR |= ADC_CCR_DELAY_3; //9*TADC_CLK
}


//-------------------------------------------------------------------------------------------
// 	ADC Input Channel Initialization
//  One external input channel: PA1 (ADC12_IN6)
//-------------------------------------------------------------------------------------------
void ADC_Pin_Init(void){	
	// 1. Enable the clock of GPIO Port A
	RCC->AHB2ENR |=   RCC_AHB2ENR_GPIOAEN;
	
	// 2. Configure GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	//    Configure PA1 (ADC12_IN6) as Analog mode
	GPIOA->MODER |=  0b11UL<<(2*1);  
	
	// 3. Configure GPIO Analog Switch Control Register (ASCR)
	//    -0: Disconnect analog switch to the ADC input (reset state)
	//    -1: Connect analog switch to the ADC input
	GPIOA->ASCR |= 1UL<<(1*1);
}

//--------------------------------------------------------------------------------------------------
// Initialize ADC: Configure ADC clock, input mode (single-ended/differential), conduct calibration, 
// configure input channel, data resolution and alignment, regular/injected channel sequence, 
// conversion mode (single/continuous), and interrupts.
//--------------------------------------------------------------------------------------------------	
void ADC_Init(void){
	
	// 1. Disable ADC1 before further configurations: set ADC1_CR register's ADEN bit to 0 
	//    0: Disabled; 1: Enabled
	ADC1->CR &= ~ADC_CR_ADEN;  
	// 2. Enable the clock of ADC
	RCC->AHB2ENR  |= RCC_AHB2ENR_ADCEN;
	// 3. Configure the ADC clock and dual mode parameters via the ADC common control register ADC_CCR
	ADC_Common_Configuration();
	// 4. Wake up ADC1 from the deep power down mode
	ADC1_Wakeup();
	
	// Sampling time 
	ADC1->SMPR1 |= ADC_SMPR1_SMP1_2; // Sampling Time (pg.523/1903 Ref. Manual)
	// 5. Configure single-ended or differential mode for the ADC input channel via ADC1_DIFSEL register
	//    0: Single-ended mode; 1: Differential mode
	ADC1->DIFSEL &= ~ADC_DIFSEL_DIFSEL_6; 	// Select single-ended mode for PA1 (ADC12_IN6) 
	// 6. Start ADC1 calibration. 
	//    Calibration is preliminary to any ADC operation. It removes the offset error which may vary
	//    from chip to chip due to process or bandgap variation.
	//		The calibration factor to be applied for single-ended input conversions is different from the
	//    factor to be applied for differential input conversions, so it is essential to do calibration
	//		after configuring ADCx_DIFSEL register in Step 5. 
	ADC1->CR |=  ADC_CR_ADCAL; //Start ADC1 calibration 
	while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL); //wait till calibration is done
	
	// 7. After calibration is done, the ADCx module can be enabled for the subsequent configurations.
	//    Set ADEN bit in ADC1_CR register to enable ADC1.
	ADC1->CR |= ADC_CR_ADEN;  
	// 8. Invoke ADC_Pin_Init() to initilize ADC input channel(s).
	//    In this sample, one analog input channel PA1 (ADC12_IN6) is used as Input Channel 6 for ADC1. 
	ADC_Pin_Init();
	
	// 9. Configure the ADC data resolution and alignment via ADC Configuration Register ADCx_CFGR
	//    RES[1:0]- Data Resolution (00 = 12-bit, 01 = 10-bit, 10 = 8-bit, 11 = 6-bit)
	//    ALIGN - Data Alignment (0 = Right alignment, 1 = Left alignment)
	ADC1->CFGR &= ~ADC_CFGR_RES; //select 12-bit resolution     	
	ADC1->CFGR &= ~ADC_CFGR_ALIGN; //select right alignment  	 
		
	// 10. Set up the ADC Regular Sequence in ADCx_SQR registers
	//     First, define the sequence length in ADC1_SQR1 register, L[3:0] field:
	//     -0000: 1 conversion
	//		 -0001: 2 conversions
	//      ...
	//     -1111: 16 conversions
	ADC1->SQR1 &= ~ADC_SQR1_L;     // 0000: Set sequence length to 1 conversion.
	//     Next, assign the channel number for each conversion in the sequence.
	//     In this sample, only the 1st conversion channel SQR1[4:0] is used
	ADC1->SQR1 &= ~ADC_SQR1_SQ1;	// Clear SQR1[4:0] (bits 10:6) in ADC1_SQR1
	ADC1->SQR1 |=  ( 6U << 6 );   // Set channel 6 (connected to PA1) for the 1st conversion
	
	// 11. Set ADC single/continuous conversion mode for regular conversions via ADCx_CFGR, CONT (bit 13):
	//     0: single conversion; 1: continuous conversion
	ADC1->CFGR &= ADC_CFGR_CONT;  // Enable single conversion mode (pg.523/1903 Ref. Manual)
	

	// 12. Configure external trigger for regular channels via ADCx_CFGR, EXTEN[1:0] field
	//   -00: Hardware Trigger detection disabled, software trigger detection enabled
	//   -01: Hardware Trigger with detection on the rising edge
	//   -10: Hardware Trigger with detection on the falling edge
	//   -11: Hardware Trigger with detection on both the rising and falling edges
	ADC1->CFGR &= ~ADC_CFGR_EXTEN; //select software trigger
	
	// 13. Set up ADC interrupts
	//     Enable specific type of ADC interrupts via ADC interrupt enable register (ADCx_IER)
	ADC1->IER |= ADC_IER_EOC;  // Enable End of Regular Conversion interrupt
	NVIC_EnableIRQ(ADC1_2_IRQn); // Enable the ADC1_2 interrupt (shared by ADC1 and ADC2) in NVIC 
	
	// 14. Wait till ADC is ready to accept conversion.
	//     The ADRDY bit in ADC1_ISR is set by hardware after the ADC has been enabled (bit ADEN=1) and when the ADC
	//     reaches a state where it is ready to accept conversion requests.
	while((ADC1->ISR & ADC_ISR_ADRDY) == 0); 
}


//-------------------------------------------------------------------------------------------
// 	Interrupt Handler for ADC1
//	Note: ADC1 and ADC2 share an interrupt vector 'ADC1_2_IRQHandler'
//-------------------------------------------------------------------------------------------
void ADC1_2_IRQHandler(void){    
	
	// Check if the interrupt is triggered by ADC1 End of Conversion (EOC) 
	if ((ADC1->ISR & ADC_ISR_EOC) == ADC_ISR_EOC) {
		
	// Clear the interrupt by writing 1 to it or by reading the corresponding ADC1_DR register
  ADC1->ISR |= ADC_ISR_EOC;
	// Read the sampled data from ADC1_DR and store it in the global variable 'adc_result'
	adc_result = ADC1->DR;
	}

}





