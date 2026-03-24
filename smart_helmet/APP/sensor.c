// stm32与asr-por通信

#include "sensor.h"

// 报警取消标志位
bool alarm_cancel_flag = 0;
// 测量体征标志位
// bool measure_flag = 0;
// 报警取消计数器
uint8_t alarm_cancel_count = 0;

// 使用HAL 库里专门提供的串口接收完成的回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 判断是哪一个串口触发的回调。
    // STM32 可能有多个串口，比如 USART1、USART2 等。
    // 这个判断是为了确保这段逻辑只响应 USART1 的接收完成。
    if (huart->Instance == USART1)
    {
        // 记录当前接收到这个字节的系统时间，可用于判断数据帧是否结束
        uart_rx_ticks = uwTick; // 相当于uart_rx_ticks = HAL_GetTick();

        // 将接收缓冲区的索引加1。
        uart_rx_index++;
        
        // 接收到这个数据之后 重新开启下一次中断接收。
        HAL_UART_Receive_IT(&huart1, &uart_rx_buffer[uart_rx_index], 1);
    }
}

/**
 * @brief   处理语音识别数据函数
 */
void asr_pro_task(void)
{
    // 如果接收索引为0，说明没有数据需要处理，直接返回
    if (uart_rx_index == 0)
        return;

    // 如果从最后一次接收到数据到现在已经超过10ms
    if (uwTick - uart_rx_ticks > 10) // 100ms内没有收到新的数据
    {
        uart_rx_buffer[uart_rx_index] = '\0'; // 添加字符串结束符

        // 对接收到的数据进行判断
        if (strcmp((char *)uart_rx_buffer, "good\r\n") == 0) // 判断是否接收到了 "good"
        {
            // 执行相应的操作
            alarm_cancel_flag = 1;   // 设置报警取消标志位
            alarm_cancel_count = 10; // 设置报警取消有效时间（10*1000ms=10s，假设执行周期为1000ms）

            // 串口调试
            // printf("alarm_cancel_flag = %d\r\n", alarm_cancel_flag);
            // printf("larm_cancel_count = %d\r\n", alarm_cancel_count);
        }
        else if (strcmp((char *)uart_rx_buffer, "measure\r\n") == 0) // 如果是接收了"measure"
        {
            measure_flag = 1; // 设置测量体征标志位 开始执行一次MAX30102心率血氧传感器模块的执行函数,进行心率血氧的测量 如果还需要测量 则还需要继续说

            // 串口调试
            // printf("measure_flag = %d\r\n", measure_flag);
        }

        // 清空接收缓冲区，并将接收索引置零
        memset(uart_rx_buffer, 0, uart_rx_index);
        uart_rx_index = 0;

        // 将UART接收缓冲区指针重置为接收缓冲区的起始位置
        huart1.pRxBuffPtr = uart_rx_buffer;
    }
}

/**
 * @brief   向语音识别模块发送标志函数
 */
void sensor_task(void)
{
    // 如果报警取消标志位为0，说明没有报警取消操作，可以正常发送传感器数据
    if (!alarm_cancel_flag)
    {
        // 根据不同的传感器状态发送相应的信息
        // 跌倒的优先级最高
        if (fall_flag)
        {
            printf("fall"); // 发送跌倒报警信息
        }
        else if (collision_flag)
        {
            printf("collision"); // 发送撞击报警信息
        }
        else if (density_flag)
        {
            printf("density"); // 发送浓度过高报警信息
        }
        else if (heartrate_flag)
        {
            printf("heartrate"); // 发送心率异常报警信息
            heartrate_flag = 0;  // 重置心率异常标志位 避免多次重复报警
            spo2_flag = 0;       // 重置血氧异常标志位 避免多次重复报警
        }
        else if (spo2_flag)
        {
            printf("spo2");     // 发送血氧异常报警信息
            heartrate_flag = 0; // 重置心率异常标志位 避免多次重复报警
            spo2_flag = 0;      // 重置血氧异常标志位 避免多次重复报警
        }
    }
    else
    {
        // 如果报警取消标志位不为0，说明正在进行报警取消操作
        if (--alarm_cancel_count == 0)
        {
            // 报警取消计数器减到0时，重置报警取消标志位
            alarm_cancel_flag = 0;
        }
    }
}
