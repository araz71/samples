

/* Includes */
#include "stm32f0xx.h"

#define ENABLE_MEASURE	TIM3->CR1 |= TIM_CR1_CEN;
#define DISABLE_MEASURE	TIM3->CR1 &= TIM_CR1_CEN;

uint8_t dist_state = 0;
uint64_t dist_cntr = 0;

void TIM3_IRQHandler() {
	TIM3->SR = 0;
	dist_cntr++;
}

void EXTI4_15_IRQHandler() {
	EXTI->PR |= EXTI_PR_PR7;
	if (GPIOA->IDR & (1 << 7)) {
		dist_cntr = 0;
		ENABLE_MEASURE;
	} else {
		float dist = dist_cntr * 1.7;
		DISABLE_MEASURE;
	}
}

uint64_t tick_cntr = 0;
void SysTick_Handler() {
	tick_cntr++;
}

int main() {

	SysTick_Config(8000);

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA->MODER |= GPIO_MODER_MODER4_0;
	GPIOA->BSRR = GPIO_BSRR_BR_4;

	GPIOA->MODER |= GPIO_MODER_MODER6_0;
	GPIOA->BRR = GPIO_BRR_BR_4;

	TIM3->ARR = 800;
	TIM3->CR1 = TIM_CR1_ARPE;
	TIM3->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM3_IRQn);

	SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI7_PA;
	EXTI->IMR |= EXTI_IMR_MR7;
	EXTI->FTSR |= EXTI_FTSR_TR7;
	EXTI->RTSR |= EXTI_RTSR_TR7;
	NVIC_EnableIRQ(EXTI4_15_IRQn);

	while (tick_cntr < 1000);
	tick_cntr = 0;
	while(1) {
		if (tick_cntr >= 500) {
			tick_cntr = 0;
			GPIOA->BSRR |= GPIO_BSRR_BR_6;
			while (tick_cntr < 5);
			GPIOA->BRR |= GPIO_BSRR_BR_6;
			tick_cntr = 0;
		}
	}
}
