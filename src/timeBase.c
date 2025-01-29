#include "timeBase.h"
#include "stm32f4xx.h"

#define PLL_M 25
#define PLL_N 400
#define PLL_P 1  // for pllp = 4


void timeBase_init(void)
{
  // Activate HSE and wait it to be ready.
  RCC->CR |= RCC_CR_HSEON;
  while (!(RCC->CR & RCC_CR_HSERDY));

  // Enable Flash prefetch and set 3 wait state
  FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_3WS;

  // Set proper division factor, use CUBEIDE or CUBEMX for values
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

  // APB1 division facor is 2, so APB1 freq is 50Mhz
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

  // APB2 division factor is 1, so APB2 freq is 100Mhz
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

  // Set PLL configurations M:25 N:400 P:4
  RCC->PLLCFGR = (PLL_M <<0) | (PLL_N << 6) | (PLL_P <<16) | (RCC_PLLCFGR_PLLSRC_HSE);

  // Enable PLL and wait it to be ready.
  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY));

  // Select PLL as system source clock
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while (!(RCC->CFGR & RCC_CFGR_SWS_PLL));

  SystemCoreClockUpdate();

  /// AHB = 100 Mhz
  /// AP1 = 50 Mhz
  /// AP1 timer = 100 Mhz
  /// AP2 = 100 Mhz
  /// AP2 timer = 100 Mhz

  // Init Systick timer for 1ms interval
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;    // Enable counter
  SysTick->LOAD = 12500000;                    // Update Reload for 1 sec
  SysTick->VAL = 0;                            // Reset counter
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;   // Enable interrupt
  SysTick->CTRL &= ~(1 << 2);                  // Systick clock source = AHB / 8 => 100 / 8 = 12 Mhz
  NVIC_EnableIRQ(SysTick_IRQn);                // Enable interrupt from NVIC
  NVIC_SetPriority (SysTick_IRQn, 15);          // Set interrupt priority to low
}
