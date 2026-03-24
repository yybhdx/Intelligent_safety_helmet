#ifndef __MAX30102_APP__
#define __MAX30102_APP__

#include "bsp_system.h"

// 若觉得测量的准度不够 可以通过修改下面的三个数测量准度
#define HEART_RATE_COMPENSATION 10 // 心率补偿值，每次补偿固定的心率值
#define WINDOW_SIZE 20 // 滑动窗口大小
#define ALPHA 0.05      // 低通滤波的滤波系数

#define HEART_RATE_COMPENSATION 10 // 心率补偿值，每次补偿固定的心率值
#define WINDOW_SIZE 20 // 滑动窗口大小
#define ALPHA 0.05      // 低通滤波的滤波系数

// 定义常量和数据结构
#define MAX_BRIGHTNESS 255
#define INTERRUPT_REG 0X00
#define BUFFER_LENGTH 500 // 数据缓存长度

// MAX30102 数据结构
typedef struct
{
  uint32_t ir_buffer[BUFFER_LENGTH];  // 红外LED数据（用于血氧计算）
  uint32_t red_buffer[BUFFER_LENGTH]; // 红色LED数据（用于心率计算）
  int32_t spO2;                       // 血氧饱和度
  int8_t spO2_valid;                  // 血氧有效性指示
  int32_t heart_rate;                 // 心率
  int8_t heart_rate_valid;            // 心率有效性指示
  int32_t min_value;                  // 信号最小值
  int32_t max_value;                  // 信号最大值
  int32_t prev_data;                  // 上一数据点
  int32_t brightness;                 // 信号亮度（用于心率计算）
  uint32_t buffer_length;             // 数据缓冲区长度
} MAX30102_Data;

void MAX30102_Read_Data(void);
void Calculate_Heart_Rate_and_SpO2(void);
void Update_Signal_Min_Max(void);
extern MAX30102_Data max30102_data;
void Process_And_Display_Data(void);

void max30102_task(void);

#endif


