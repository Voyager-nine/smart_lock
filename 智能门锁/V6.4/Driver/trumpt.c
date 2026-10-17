#include "debug.h"

unsigned char trumpet_flag=0;

/*num表示播放语音模块中存储的第num个语音*/
void trumpet(unsigned char num)
{


   USART_SendData(UART6, 0X7E);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, 0X05);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, 0X41);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, 0X00);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, num);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, 0x44^num);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
   USART_SendData(UART6, 0xef);
   while(USART_GetFlagStatus(UART6,USART_FLAG_TC)==0);
}
