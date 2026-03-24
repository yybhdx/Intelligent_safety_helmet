#ifndef _BSP_SYSTEM_H
#define _BSP_SYSTEM_H

#include "main.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h" //用于支持bool类型
#include "math.h"

#include "scheduler.h" //任务调度

// dht11温湿度传感器相关
#include "sys.h"
#include "tim.h"
#include "dht11.h"

// MQ-2烟雾传感器模块相关
#include "adc.h" //添加adc头文件 方便mq2.c调用ADC
#include "mq2.h" //添加MQ-2烟雾传感器头文件

// mpu6050  相关(硬件I2C)
#include "i2c.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "dmpKey.h"
#include "dmpmap.h"

// MAX30102心率血氧传感器模块相关(软件I2C)
#include "myiic.h"
#include "max30102.h"
#include "max30102_app.h"
#include "algorithm.h"

// ATGM336H定位模块相关
#include "atgm336h.h"
#include "usart.h"
#include "ringbuffer.h"

// 整体功能实现模块相关
#include "sensor.h"

// ESP8266模块
#include "esp01s.h"

// ATGM336H定位模块相关
extern uint16_t uart_rx_index;
extern uint32_t uart_rx_ticks;
extern uint8_t uart_rx_buffer[1000];
extern uint8_t uart_rx_dma_buffer[1000];

extern uint32_t dma_buffer[30];

extern float ppm;           // MQ-2气体浓度
extern bool density_flag;   // 烟雾浓度异常标志位
extern bool measure_flag;   // 模块运行标志位
extern bool heartrate_flag; // 心率异常标志位
extern bool spo2_flag;      // 血氧异常标志位
extern uint8_t dis_hr;      // 显示的心率值
extern uint8_t dis_spo2;    // 显示的血氧值
extern bool fall_flag;      // 陀螺仪标志位
extern bool collision_flag; // 陀螺仪标志位
extern uint8_t humi;        // DHT11温湿度模块湿度
extern uint8_t temp;        // DHT11温湿度模块温度
extern char longitude[64];  // ATGM336H经度
extern char latitude[64];   // ATGM336H纬度
#endif
