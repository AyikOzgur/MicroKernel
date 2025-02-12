#include "stm32f4xx.h"  // Include the necessary header for SystemCoreClock

void usart2_init(uint32_t baudrate)
{
  // Enable GPIOA and USART2 clocks
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // Enable GPIOA CLOCK
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable USART2 CLOCK

  // Configure PA2 (TX) and PA3 (RX) as alternate function mode
  GPIOA->MODER &= ~(0xFU << 4); // Reset bits 4:5 for PA2 and 6:7 for PA3
  GPIOA->MODER |= (0xAU << 4);  // Set bits 4:5 for PA2 and 6:7 for PA3 to alternate mode (10)
  GPIOA->OSPEEDR |= 0x000000A0; // Set pin 2/3 to high speed mode (0b10)
  GPIOA->AFR[0] |= (0x7 << 8);  // Set AF7 (USART2) for PA2
  GPIOA->AFR[0] |= (0x7 << 12); // Set AF7 (USART2) for PA3

  uint32_t apb1_freq = 50000000;
  // Calculate USART2 baud rate register value
  uint16_t uartdiv = apb1_freq / baudrate;
  USART2->BRR = (((uartdiv / 16) << 4) | (uartdiv % 16));

  // Configure USART2 for both sender and receiver
  USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE); // Enable transmitter, receiver, and USART

  // Set data length to 8-bit, disable parity
  USART2->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);

  // Set 1 stop bit
  USART2->CR2 &= ~(USART_CR2_STOP);
}

void usart2_send_char(char c)
{
  while (!(USART2->SR & USART_SR_TXE)); // Wait until TX buffer is empty
  USART2->DR = c;
}

void usart2_send_string(const char *str)
{
  while (*str)
  {
    usart2_send_char(*str++);
  }
}

void usart2_send_data(uint8_t *data, int size)
{
  for (int i = 0; i < size; i++)
  {
    usart2_send_char(data[i]);
  }
}
