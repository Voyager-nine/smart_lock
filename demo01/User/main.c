#include "debug.h"

unsigned int time_1000ms;
unsigned char led_flag;

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//定时器3快速中断
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口3快速中断
/*如何配置定时器的重装载值和分频值
 *主频 96MHz
 *96分频后为1MHz
 *
 *如果想配置1ms的定时器，则频率应为1000hz
 *即96分频后每计数到1000就归零，此时频率就为1000hz
 *所以arr=1000 psc=96-1
 */

//GPIO初始化
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};//定义GPIO初始化的结构体，用于传参

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//打开APB2总线上GPIOC的时钟 可更改项：总线、GPIO分组
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//GPIO翻转速度
    GPIO_Init(GPIOC, &GPIO_InitStructure);//注册
}

//定时器3初始化函数
void Tim3_Init(u16 arr,u16 psc)//arr：自动重装载值 psc：分频值
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;//定义配置定时器的结构体
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitStruct.TIM_ClockDivision=0;//时钟分割
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;//设置为向上计数
    TIM_TimeBaseInitStruct.TIM_Period=arr;//自动重装载值，5000为500ms
    TIM_TimeBaseInitStruct.TIM_Prescaler=psc;//设置分频值

    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
    TIM_ITConfig(TIM3, TIM_IT_Update|TIM_IT_Trigger, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)!=RESET)
    {
        if(++time_1000ms == 1000)
        {
            time_1000ms = 0;
            led_flag ^= 1;
            if(led_flag == 0)
                GPIO_SetBits(GPIOC, GPIO_Pin_2);
            else
                GPIO_ResetBits(GPIOC, GPIO_Pin_2);
        }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}


//串口3初始化
void Usart3_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

    /* USART3_TX -> PB10 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART3_RX -> PB11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   // 或 GPIO_Mode_IPU
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

//串口3中断
void USART3_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        temp=USART_ReceiveData(USART3);
        temp += 1;
        printf("%c\r\n",temp);
    }
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组
	SystemCoreClockUpdate();//时钟配置

	Delay_Init();//延时函数初始化

//	USART_Printf_Init(115200);//串口打印初始化
//	printf("SystemClk:%d\r\n",SystemCoreClock);
//	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
//	printf("This is printf example\r\n");

//	printf("Hello World\r\n");//必须加上 \r\n，否则电脑上无显示，应当是printf函数要求的
	GPIO_Toggle_INIT();//初始化GPIOC PIN2
	Tim3_Init(1000,96-1);
	Usart3_Init();
	printf("Hello World\r\n");
	while(1)
    {
//	    GPIO_SetBits(GPIOC, GPIO_Pin_2);
//	    Delay_Ms(500);
//	    GPIO_ResetBits(GPIOC, GPIO_Pin_2);
//	    Delay_Ms(500);

	}
}

