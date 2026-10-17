#include "mydefine.h"
#include "pic.h"
#include "scheduler.h"

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uart2_rx_data = (uint8_t)USART_ReceiveData(USART2);
        uart2_rx_flag = 1;
    }
}

void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uart3_rx_data = (uint8_t)USART_ReceiveData(USART3);
        uart3_rx_flag = 1;
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    Millis_Init();
    USART_Printf_Init(115200);
    Usart2_Init();

    TIM2_PWM_Init();//定时器2初始化 输出PWM控制舵机
    lock_proc();//舵机初始化 默认上锁

    LCD_Init();//lcd屏幕初始化
    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色

    //显示汉字，需要提前准备字库
    LCD_ShowChinese_String(32, 50, "正在启动", BLUE, WHITE, 16, 0);

    //显示进度条
    unsigned char i;
    for(i=0;i<128;i++)
    {
        LCD_DrawLine(i, 100, i, 127, RED);
        Delay_Ms(10);
    }

    //此处不用重新刷新背景，是因为显示汉字时有对背景进行设置？
    LCD_ShowChinese_String(32, 50, "启动成功", BLUE, WHITE, 16, 0);
    Delay_Ms(500);

    //显示图片
    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色
    LCD_ShowPicture(0, 0, 128, 119, gImage_welcome_to_home + 8);

    Delay_Ms(1000);

    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色
    LCD_ShowPicture(0, 0, 128, 128, gImage_lock_disp + 8);

    LCD_Fill(20, 30, 107, 57, LGRAY);

    voice_init();
    voice_play(VOICE_TRACK_LOCKED);

    key_init();
    Usart2_Init();
    scheduler_init();

    while(1)
    {
        scheduler_run();
    }
}

