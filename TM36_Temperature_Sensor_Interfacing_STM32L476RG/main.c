#include "stm32l476xx.h"
#include "sensor_ADC_driver.h"
#include "usart2_driver.h"
#include "string.h"
#include "stdio.h"

char tempC_buffer[10]; // temperature buffer
uint32_t temperature_C; // temperature in Celsius
float voltage_raw; // raw voltage value from sensor
float voltage; // voltage in mV (stored after conversion)


void send_string_via_usart(const char *str) {
  while (*str) {
      while (!(USART2->ISR & USART_ISR_TXE)); // Wait until TXE (Transmit Data Register Empty)
      USART2->TDR = *str++;                // Load the next character into TDR
  }
}


int main(void){
	
	
	const char *msg = "Temperature Sensor Initialized.\n\r";

	// Initialize ADC: Set up ADC1 for sampling from external input channel PA1 (ADC1_IN6). 
	// Configure for 12-bit resolution, right data alignment, single-ended, continuous mode, 
	// and interrupt at the end of every conversion.
	ADC_Init();
  
	// Initialize UART
	USART2_Init();
	
	// Initial message on UART
	send_string_via_usart(msg);
		
	while(1){
		
		// Start ADC conversion
		ADC1->CR |= ADC_CR_ADSTART;
		
		// Calculate temperature
		voltage_raw = adc_result;
	  voltage = (0.00081 * voltage_raw);
	  temperature_C = (voltage - 0.5)*100;
		
		//format the temperature and send over UART
		sprintf(tempC_buffer, "%u\n\r", temperature_C);
		send_string_via_usart(tempC_buffer);
		
		
		// delay 
		int i;
		for(i=0; i<=100000; i++);
		
		// Stop ADC conversion
		ADC1->CR &= ~ADC_CR_ADSTART;	
	}
	
} 



