#include <stm32f10x.h>

unsigned long TIM2_interrupts=0;

void rcc_init()
{
	RCC->CR|=RCC_CR_HSEON;
	RCC->CR &= ~RCC_CR_PLLON; //OFF pll before change
	while((RCC->CR & RCC_CR_HSERDY)==0);
	
	RCC->CR &= ~RCC_CR_PLLON;//CLEAR
	RCC->CFGR &= ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC);//CLEAR
	RCC->CFGR |= (RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC_HSE);
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_HSERDY)); 
	
	RCC->CFGR &=~RCC_CFGR_SW;//CLEAR
	RCC->CFGR |=RCC_CFGR_SW_PLL;
	while((RCC->CFGR & RCC_CFGR_SWS)!=RCC_CFGR_SWS_PLL);
	
}

void TIM2_init()
{
	RCC->APB1ENR|=RCC_APB1ENR_TIM2EN; //SHINA WITH TIM2 ON
	
	TIM2->PSC=7600-1;
	TIM2->ARR=10000-1;
	
	TIM2->DIER|=TIM_DIER_UIE; //interapt enable with tim2 signal UIE-update interrupt enable
	NVIC_EnableIRQ(TIM2_IRQn);
	
	TIM2->CR1|=TIM_CR1_CEN; //enable timer
}

void TIM2_IRQHandler()
{
	TIM2->SR &= ~TIM_SR_UIF;//IMPORTANT CLEAR FLAG TO EXIT HANDLER THEN!
	TIM2_interrupts++;
	
	if (GPIOA->ODR & (1 << 5)) 
	{
    GPIOA->BSRR = (1 << (5 + 16)); 
	} 
	else {
			GPIOA->BSRR = (1 << 5);        
	}
}

int main()
{
	rcc_init();
	SystemCoreClockUpdate();
	TIM2_init();
	
	//led EXTERNAL
	RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;
	GPIOC->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
	GPIOC->CRH |=~(GPIO_CRH_CNF9_0|GPIO_CRH_CNF9_1);
	GPIOC->CRH |=(GPIO_CRH_MODE9_1);
	
		//led internal
	RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
	GPIOA->CRL |=(1<<4*5);
	
	while(1)
	{
		GPIOC->BSRR|=GPIO_BSRR_BR9;
		//GPIOA->BSRR|=GPIO_BSRR_BS5;
	}
	
	
	return 0;
}