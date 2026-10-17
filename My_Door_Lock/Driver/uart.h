#ifndef DRIVER_UART_H_
#define DRIVER_UART_H_

#include "mydefine.h"

#define UART2_BAUDRATE 115200
#define UART2_TX_PORT GPIOA
#define UART2_TX_PIN GPIO_Pin_2
#define UART2_RX_PORT GPIOA
#define UART2_RX_PIN GPIO_Pin_3
#define UART2_FRAME_LEN 12

#define UART3_BAUDRATE 9600
#define UART3_TX_PORT GPIOB
#define UART3_TX_PIN GPIO_Pin_10
#define UART3_RX_PORT GPIOB
#define UART3_RX_PIN GPIO_Pin_11

extern volatile uint8_t uart3_rx_data;
extern volatile uint8_t uart3_rx_flag;

void Usart2_Init(void);
void uart2_receive_byte(uint8_t data);
uint8_t uart2_read_frame(uint8_t frame[UART2_FRAME_LEN]);
void uart2_discard_frame(void);
void Usart3_Init(void);

#endif /* DRIVER_UART_H_ */
