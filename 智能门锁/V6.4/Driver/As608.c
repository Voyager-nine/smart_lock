#include "debug.h"
#include "Uart.h"
#include "As608.h"
#include  "lock.h"
#include  "trumpt.h"
u_int16_t head[6]={0XEF,0X01,0XFF,0XFF,0XFF,0XFF};
u_int16_t cmd[20]={};
extern u16 Usart2_Rc_Buff[20];


//录入指纹
void input_fingerprint(u8 num)
{


        PS_GetImage();
        Delay_Ms(100);//等待as608响应
        PS_GenChar(1);
        Delay_Ms(100);
        PS_GetImage();
        Delay_Ms(100);
        PS_GenChar(2);
        Delay_Ms(100);
        PS_RegModel();
        Delay_Ms(100);
        PS_StoreChar(num);
        Delay_Ms(100);
}

/*比对指纹*/
void Contrast_fingerprint()
{


    PS_GenChar(1);
    Delay_Ms(1000);
     PS_Search(100);
      Delay_Ms(1000);
}

/*查找指纹函数*/
void PS_Search(u8 num)
{
        /*控制命令*/
        cmd[0]=0x01;
        cmd[1]=0x00;
        cmd[2]=0x08;
        cmd[3]=0x04;
        cmd[4]=0x01;
        cmd[5]=0x00;
        cmd[6]=0x00;
        cmd[7]=0x00;
        cmd[8]=num;
        cmd[9]=0x00;
        cmd[10]=cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]+cmd[7]+cmd[8]+cmd[9];

        Usart2_Send_datapack(head, 6);//发送6位包头
        Usart2_Send_datapack(cmd, 11);//发送命令
}

/*as608等待函数*/
void PS_Wait()
{
   u8 temp=99;
   while(temp)
   {
   temp=Usart2_Rc_Buff[1];
   }
}

/*从传感器上读入图像存于图像缓冲区*/
void PS_GetImage()
{
     cmd[0]=0x01;
     cmd[1]=0x00;
     cmd[2]=0x03;
     cmd[3]=0x01;
     cmd[4]=0x00;
     cmd[5]=0x05;
     Usart2_Send_datapack(head, 6);
     Usart2_Send_datapack(cmd, 6);
}


/*根据原始图像生成指纹特征存于 CharBuffer1 或 CharBuffer2,由参数block控制*/
void PS_GenChar(u16 block)
{
    cmd[0]=0x01;
    cmd[1]=0x00;
    cmd[2]=0x04;
    cmd[3]=0x02;
    cmd[4]=block;
    cmd[5]=0x00;
    cmd[6]=0x07+block;
    Usart2_Send_datapack(head, 6);
    Usart2_Send_datapack(cmd, 7);
}

/*将 CharBuffer1 与 CharBuffer2 中的特征文件合并生成模板存于
CharBuffer2 */
void PS_RegModel()
{
        cmd[0]=0x01;
        cmd[1]=0x00;
        cmd[2]=0x03;
        cmd[3]=0x05;
        cmd[4]=0x00;
        cmd[5]=0x09;

    Usart2_Send_datapack(head, 6);
    Usart2_Send_datapack(cmd, 6);
}


/*从 flash 指纹库中读取一个模板到特征缓冲区 */
void PS_StoreChar(u8 num)
{
    cmd[0]=0x01;
    cmd[1]=0x00;
    cmd[2]=0x06;
    cmd[3]=0x06;
    cmd[4]=0x01;
    cmd[5]=0x00;
    cmd[6]=num;
    cmd[7]=0x00;
    cmd[8]=num+1+6+6+1;

    Usart2_Send_datapack(head, 6);
    Usart2_Send_datapack(cmd, 9);

}

/*初始化*/
void As608_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
