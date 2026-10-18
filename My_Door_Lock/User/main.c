#include "mydefine.h"
#include "pic.h"
#include "scheduler.h"

volatile u8 uart7_rec_string[20]={0};
volatile u8 uart7_rec_index;
volatile u8 uart7_rec_tick;
volatile u8 uart7_rec_ready;

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uart2_receive_byte((uint8_t)USART_ReceiveData(USART2));
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

void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void)
{
    if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
    {
        uart7_receive_byte((u8)USART_ReceiveData(UART7));
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    Millis_Init();
    USART_Printf_Init(115200);

    TIM2_PWM_Init();//定时器2初始化 输出PWM控制舵机
    lock_proc();//舵机初始化 默认上锁

    LCD_Init();//lcd屏幕初始化
    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色

    //显示汉字，需要提前准备字库
    LCD_ShowChinese(32, 4, (u8 *)"\xD5\xFD\xD4\xDA\xC6\xF4\xB6\xAF", BLUE, WHITE, 16, 0);

    //显示进度条
    unsigned char i;
    for(i=0;i<128;i++)
    {
        LCD_DrawLine(i, 100, i, 127, RED);
        Delay_Ms(10);
    }

    //此处不用重新刷新背景，是因为显示汉字时有对背景进行设置？
    LCD_ShowChinese(32, 4, (u8 *)"\xC6\xF4\xB6\xAF\xB3\xC9\xB9\xA6", BLUE, WHITE, 16, 0);
    Delay_Ms(500);

    //显示图片
    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色
    LCD_ShowPicture(0, 0, 128, 119, gImage_welcome_to_home + 8);

    Delay_Ms(1000);

    LCD_Fill(0, 0, 127, 127, WHITE);//填充背景为白色
    LCD_ShowPicture(0, 0, 128, 128, gImage_lock_disp + 8);

    LCD_Fill(20, 30, 107, 57, LGRAY);

    lcd_prompt(LCD_PROMPT_LOCKED);

    key_init();
    Usart2_Init();
    as608_init();
    scheduler_init();

    PS_GetImage();
    u8 uart7_frame[20];
    u8 uart7_frame_len;
    while(1)
    {
        uart7_frame_len = uart7_read_frame(uart7_frame);
        if(uart7_frame_len)
            usart1_send_string(uart7_frame, uart7_frame_len);
        scheduler_run();
    }
}

