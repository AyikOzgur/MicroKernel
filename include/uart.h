#pragma once

void usart2_init(uint32_t baudrate);
void usart2_send_char(char c);
void usart2_send_string(const char *str);
void usart2_send_data(uint8_t *data, int size);
