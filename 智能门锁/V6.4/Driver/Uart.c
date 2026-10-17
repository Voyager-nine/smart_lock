#include "debug.h"
#include "stdio.h"
#include "string.h"

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Usart3_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);


    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;



    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);



     NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

     USART_Cmd(USART3, ENABLE);
}

void Uart8_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;//4TX 5RX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;

    USART_Init(UART8, &USART_InitStructure);

    USART_ITConfig(UART8, USART_IT_RXNE,ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART8_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UART8, ENABLE);
}

void Uart6_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;

    USART_Init(UART6, &USART_InitStructure);

    USART_ITConfig(UART6, USART_IT_RXNE,ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UART6, ENABLE);
}

void Uart7_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//2TX 3RX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;

    USART_Init(UART7, &USART_InitStructure);

    USART_ITConfig(UART7, USART_IT_RXNE,ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UART7, ENABLE);
}


void screen_printf(const char *str)
{
    while(*str!='\0')
    {
        USART_SendData(UART7, *str);
        while(USART_GetFlagStatus(UART7,USART_FLAG_TC)==0);
        str++;
    }

    /*串口屏通信包尾*/
      USART_SendData(UART7, 0xff);
      while(USART_GetFlagStatus(UART7,USART_FLAG_TC)==0);
      USART_SendData(UART7, 0xff);
      while(USART_GetFlagStatus(UART7,USART_FLAG_TC)==0);
      USART_SendData(UART7, 0xff);
      while(USART_GetFlagStatus(UART7,USART_FLAG_TC)==0);
}




//P2 Tx
void Usart2_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);


    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);



     NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

     USART_Cmd(USART2, ENABLE);
}


void Usart2_Send_datapack( u_int16_t *data, u_int8_t len  )
{
  while(len)
  {

      USART_SendData(USART2, *data++);
      while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==0);
      len--;
  }
}


u8 state;
u16 Usart2_Rc_Buff[20]={0};


/*指纹接收函数*/
void USART2_IRQHandler(void)
{
   u16 temp=0x00;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        temp=USART_ReceiveData(USART2);
        switch(state)
        {
        case 0:
           if(temp==0xef){state++;}
        break;
        case 1:
           if(temp==0x01){state++;}
           else{state=0;}
        break;
        case 2:
            if(temp==0xff){state++;}
            else{state=0;}
        break;
        case 3:
            if(temp==0xff){state++;}
            else{state=0;}
        break;
        case 4:
            if(temp==0xff){state++;}
            else{state=0;}
        break;
        case 5:
            if(temp==0xff){state++;}
            else{state=0;}
        break;
        case 6://包标识
            Usart2_Rc_Buff[0]=temp;
            state++;

        break;
        case 7://包长度
            state++;
        break;
        case 8://包长度
            Usart2_Rc_Buff[4]=temp;
            state++;
        break;
        case 9:
            Usart2_Rc_Buff[1]=temp;

            state++;
        break;
        case 10://页码

            state++;
        break;
        case 11://页码

            state++;
        break;
        case 12://得分

            state++;
        break;
        case 13://得分
            Usart2_Rc_Buff[2]=temp;
            state++;
        break;
        case 14:
            Usart2_Rc_Buff[3]=temp;


            state=0;
        break;
        }

    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}




void Zigbee_Send(u8 addr,u8* string)
{
    u8 addr1=0x00,addr2=0x00;
    switch(addr)
    {
        case 0:
            addr1=0x00;
            addr2=0x00;
        break;
        case 1:
              addr1=0xA3;
              addr2=0x6E;
              //路由1，粮仓
        break;
        case 2:
              addr1=0x0F;
              addr2=0x68;
              //路由2，厨房
        break;
        case 3:
              addr1=0x23;
              addr2=0x24;
              //终端1，靠近大门的次卧
        break;
        case 4:
            addr1=0x44;
            addr2=0xB2;
             //终端2，大门
    }


    USART_SendData(UART8, 0x55);//包头
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);

    USART_SendData(UART8, 0x00);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, 0x44);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//命令

    USART_SendData(UART8, 0x00);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, 0x10);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//长度


    u8 temp=0x00;
    temp=0x00^0x44^0x00^0x10^0x02^addr1^addr2^0x01^0x01^0x0A^string[0]^string[1]^string[2]^string[3]^string[4]^string[5]^string[6]^string[7]^string[8]^string[9];
    USART_SendData(UART8, temp);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//校验


    USART_SendData(UART8, 0x02);//短地址发送方式
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);

    USART_SendData(UART8, addr1);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, addr2);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//短地址


    USART_SendData(UART8, 0x01);//源端口
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, 0x01);//目标端口
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);

    USART_SendData(UART8, 0x0A);//数据位长度
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);


    USART_SendData(UART8, string[0]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[1]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[2]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[3]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[4]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[5]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[6]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[7]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[8]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);
    USART_SendData(UART8, string[9]);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//数据

    USART_SendData(UART8, 0xAA);
    while(USART_GetFlagStatus(UART8,USART_FLAG_TC)==0);//包尾

}






