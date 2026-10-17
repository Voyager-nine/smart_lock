#ifndef DRIVER_UART_H_
#define DRIVER_UART_H_

#include "mydefine.h"

#define UART3_BAUDRATE 9600
#define UART3_TX_PORT GPIOB
#define UART3_TX_PIN GPIO_Pin_10
#define UART3_RX_PORT GPIOB
#define UART3_RX_PIN GPIO_Pin_11

extern volatile uint8_t uart3_rx_data;
extern volatile uint8_t uart3_rx_flag;

void Usart3_Init(void);

#endif /* DRIVER_UART_H_ */
