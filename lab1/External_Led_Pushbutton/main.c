/******************************************************
Description : External LED and Push Button interfacing.
              External LED connected to PC1	and Push
							button is connected to PC0 in external
							PULL-UP configuration.
*******************************************************/

#include <stm32l4xx.h>

#define LED_PIN 1
#define BUTTON_PIN	0

void configure_LED_pin(void);
void configure_Button_pin(void);

/* Configure the LED; PC1 */
void configure_LED_pin(void){
	
	/* GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset) */
	GPIOC->MODER &= ~(3UL<<(2*LED_PIN));
	GPIOC->MODER |= 1UL<<(2*LED_PIN);  		
	/* GPIO Output Type: Output push-pull (0, reset), Output open drain (1)*/
	GPIOC->OTYPER &= ~(1UL<<LED_PIN);
	/* GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)*/
	GPIOC->PUPDR  &= ~(3UL<<(2*LED_PIN)); /*No pull-up, pull-down as it is external circuit*/

}

/* Configure the Button; PC0 */
void configure_Button_pin(void){
	
	/* GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)*/
	GPIOC->MODER &= ~(3UL<<(2*BUTTON_PIN));   
	/* GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)*/
	GPIOC->PUPDR  &= ~(3UL<<(2*BUTTON_PIN)); /*No pull-up, pull-down as it is external circuit*/
	
}

int main(void){
	
	/*initilize the clock for port C for button and LED*/
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	
	configure_LED_pin();
	configure_Button_pin();

	while(1){
		/*IDR gets cleared when clock is enable for Port C, therefore check the button pushed*/
		if((GPIOC->IDR & GPIO_IDR_ID0) == 0)
			GPIOC->ODR |= (GPIO_ODR_OD1);
		else
			GPIOC->ODR &= ~(GPIO_ODR_OD1);
	}
}

