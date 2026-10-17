# 语音模块开发

## Goal
为 BY8301-16P 语音模块 / BY8301-24QS 语音芯片设计 CH32V30x 串口驱动方案，并提供可移植到当前工程的示例代码。

## Requirements
- 阅读芯片说明书和语音模块说明书，提取通信协议、电气连接、关键时序。
- 参考驱动移植流程，优先复用现有 CH32V30x 标准外设库 USART/GPIO API。
- 给出默认不冲突的硬件资源建议和可配置替代方案。
- 提供 `Driver/by8301.c` + `Driver/by8301.h` 示例代码。

## Acceptance Criteria
- [ ] 说明 BY8301 UART 协议帧格式、校验方式、常用命令。
- [ ] 说明 CH32V30x 侧 UART/GPIO 连接方案和注意事项。
- [ ] 示例代码包含初始化、发送帧、播放、停止、设置音量、选曲播放、BUSY 读取。
- [ ] 示例代码使用当前工程标准外设库风格，不依赖 HAL 或动态内存。

## Technical Notes
- 当前工程使用 CH32V30x 标准外设库风格。
- `USART_Printf_Init()` 可能占用 USART1/USART2/USART3 中一个调试串口，需按 `debug.h` 的 DEBUG 配置避让。
- 当前 TIM2 CH1/PA0 已用于舵机 PWM，不应占用 PA0。
