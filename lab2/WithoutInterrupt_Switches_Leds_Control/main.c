/***********************************************************
Title: Interfacing of LEDs and Switches logic without 
				interrupt. 
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

int main(void){
	int i, n;
	//1. Invoke configure_LED_pin() to initialize PA5 as an output pin, interfacing with the LD2 LED.
	configure_LED_pin();
	//2. Invoke configure_Push_Button_pin() to initialize PC13 as an input pin, interfacing with the USER push button.
	configure_Push_Button_pin();
	//3. Turn on the LD2 LED
	turn_on_LED1();
	turn_on_LED2();
	// Infinite loop to toggle the LED, making it blink at a specified frequency.
	while(1){
		if((GPIOC->IDR & 1<<PC2) == 1<<PC2){ //externally pull-down(0)
				toggle_LED2();
				for(i=0;i<100000;i++);
		}
		else{
			turn_off_LED2();
		}
		if((GPIOC->IDR & 1<<PC3) == 1<<PC3){ //externally pull-up(1)
				toggle_LED1();
				for(i=0;i<100000;i++);
		}
		else{
			turn_off_LED1();
		}
			
	}
}
