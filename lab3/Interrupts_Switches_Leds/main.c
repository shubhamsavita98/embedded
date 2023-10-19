/***********************************************************
Title: Interfacing of LEDs and Switches logic using interrupt.
Description: Utilizing different GPIO ports,
						 pull up and pull down switch configuration and 
				     source and sink configuration for LEDs.
Hardware Connections(Circuit):
				LED1
				Positive Logic("Direct Drive")
				Output High, LED On
				Output Low, LED Off
				LED1
				Negative Logic("Sink Configuration")
				Output High, LED Off
				Output Low, LED On
				
				Switch(SW1)
				Positive Logic (Pull-down Config.)
				– pressed, 3.3V, digital ‘1’
				– not pressed, 0V, digital ‘0’
				Switch(SW1)
				Negative Logic (Pull-up Configuration)
				– pressed, 0V, digital ‘0’
				– not pressed, 3.3V, digital ‘1’			
************************************************************/

#include "stm32l476xx.h"

#define PB4   4	//LED1
#define PB5		5	//LED2
#define	PC2		2	//SW1
#define PC3		3	//SW2


void configure_LED_pin(){
  // 1. Enable the clock to GPIO Port B	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;   
		
	// 2. Configure GPIO Mode to 'Output': Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOB->MODER &= ~(3UL<<(2*PB4));  
	GPIOB->MODER |=   1UL<<(2*PB4);      // Output(01)
	GPIOB->MODER &= ~(3UL<<(2*PB5));  
	GPIOB->MODER |=   1UL<<(2*PB5);      // Output(01)
	
	// 3. Configure GPIO Output Type to 'Push-Pull': Output push-pull (0), Output open drain (1) 
	GPIOB->OTYPER &= ~(1<<PB4);      // Push-pull
	GPIOB->OTYPER &= ~(1<<PB5);      // Push-pull
	// 4. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOB->PUPDR  &= ~(3<<(2*PB4));  // No pull-up, no pull-down
	GPIOB->PUPDR  &= ~(3<<(2*PB5));  // No pull-up, no pull-down
}

void configure_Push_Button_pin(){
  // 1. Enable the clock to GPIO Port A	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;   
		
	// 2. Configure GPIO Mode to 'Input': Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOC->MODER &= ~(3UL<<(2*PC2));  	// Input (00)
	GPIOC->MODER &= ~(3UL<<(2*PC3));  	// Input (00)
	
	// 3. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOC->PUPDR  &= ~(3<<(2*PC2));  // No pull-up, no pull-down
	GPIOC->PUPDR  &= ~(3<<(2*PC3));  // No pull-up, no pull-down
}

// Modular function to turn on the LD2 LED.
void turn_on_LED1(){
	GPIOB->ODR |= 1 << PB4;
}
void turn_on_LED2(){
	GPIOB->ODR &= ~(1 << PB5);
}

// Modular function to turn off the LD2 LED.
void turn_off_LED1(){
	GPIOB->ODR &= ~(1 << PB4);
}
void turn_off_LED2(){
	GPIOB->ODR |= 1 << PB5;
}

// Modular function to toggle the LD2 LED.
void toggle_LED1(){
	GPIOB->ODR ^= (1 << PB4);
}
void toggle_LED2(){
	GPIOB->ODR ^= (1 << PB5);
}

void configure_EXTI2(void){

	//1. Enable the EXTI2 interrupt in NVIC using a function from CMSIS's core_cm4.h.
	NVIC_EnableIRQ(EXTI2_IRQn); 
	
	//2. Configure the SYSCFG module to link EXTI line 2 to GPIO PC2
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;							// Enable the clock to SYSCFG
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;     	// Clear the EXTI2 bits in SYSCFG's EXTICR0 register.
	SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI2_PC; 	// Set PC2 as the EXTI2 source in SYSCFG_EXTICR0.

	// 3. Enable (unmask) the EXTI2 interrupt by setting its corresponding bit in the EXTI's IMR.
	EXTI->IMR1 |= (1<<PC2);     //Interrupt Mask Register (IMR): 0 = marked, 1 = not masked (i.e., enabled)
	
	//4. Enable interrupt trigger for rising (button press) edge.
	EXTI->RTSR1 |= (1<<PC2);  //Rising trigger selection register (RTSR):0 = disabled, 1 = enabled
	//EXTI->FTSR1 |= (1<<PC2);  //Falling trigger selection register (FTSR): 0 = disabled, 1 = enabled
}

void configure_EXTI3(void){

	//1. Enable the EXTI3 interrupt in NVIC using a function from CMSIS's core_cm4.h.
	NVIC_EnableIRQ(EXTI3_IRQn); 
	
	//2. Configure the SYSCFG module to link EXTI line 3 to GPIO PC3
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;							// Enable the clock to SYSCFG
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;     	// Clear the EXTI13 bits in SYSCFG's EXTICR4 register.
	SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI3_PC; 	// Set PC13 (0010) as the EXTI13 source in SYSCFG_EXTICR4.

	// 3. Enable (unmask) the EXTI3 interrupt by setting its corresponding bit in the EXTI's IMR.
	EXTI->IMR1 |= (1<<PC3);     //Interrupt Mask Register (IMR): 0 = marked, 1 = not masked (i.e., enabled)
	
	//4. Enable interrupt trigger for falling (button press) edge.
	//EXTI->RTSR1 |= (1<<PC3);  //Rising trigger selection register (RTSR):0 = disabled, 1 = enabled
	EXTI->FTSR1 |= (1<<PC3);  //Falling trigger selection register (FTSR): 0 = disabled, 1 = enabled
}

// ISR (interrupt handler) for EXTI2. Interrupt handlers are initially defined in startup_stml476xx.s.
void EXTI2_IRQHandler(void) {  
	EXTI->PR1 |= EXTI_PR1_PIF2;
	toggle_LED1();
}

// ISR (interrupt handler) for EXTI3. Interrupt handlers are initially defined in startup_stml476xx.s.
void EXTI3_IRQHandler(void) {  
	EXTI->PR1 |= EXTI_PR1_PIF3;
	toggle_LED2();
}

int main(void){
	int i;
	//1. Invoke configure_LED_pin() to initialize PA5 as an output pin, interfacing with the LD2 LED.
	configure_LED_pin();
	//2. Invoke configure_Push_Button_pin() to initialize PC2 and PC3 as an input pin.
	configure_Push_Button_pin();
	
	configure_EXTI2();
	configure_EXTI3();
	
	while(1){
	}
}
