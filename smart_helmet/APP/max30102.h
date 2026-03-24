#ifndef __MAX30102_H
#define __MAX30102_H
#include "bsp_system.h"

// I2C协议中的写控制位值（0）。
#define I2C_WR 0 /* 写控制bit */

// I2C协议中的读控制位值（1）。
#define I2C_RD 1 /* 读控制bit */

// 芯片的I2C写地址?（8位格式，包含R/W位）。`0xAE = 0x57 << 1
#define max30102_WR_address 0xAE

// I2C写地址。
#define I2C_WRITE_ADDR 0xAE

// 芯片的I2C读地址?（8位格式）。`0xAF = 0x57 << 1
#define I2C_READ_ADDR 0xAF

// register addresses

// 1.基本控制与状态寄存器
//  中断状态 1?：读取该寄存器可以查看哪些中断被触发了（例如：数据就绪、环境光溢出等）。??（只读）?
#define REG_INTR_STATUS_1 0x00

// ?中断状态 2?：查看其他中断状态，比如温度读数就绪等。??（只读）?
#define REG_INTR_STATUS_2 0x01

// ?中断使能 1?：向该寄存器写入特定的位模式，可以启用或禁用REG_INTR_STATUS_1中对应的中断源。??（读写）?
#define REG_INTR_ENABLE_1 0x02

// ?中断使能 2?：用于启用或禁用REG_INTR_STATUS_2中的中断源。??（读写）?
#define REG_INTR_ENABLE_2 0x03

// ?FIFO写指针?：指向传感器内部FIFO数据缓冲区中下一个要写入的位置。??（读写）?
#define REG_FIFO_WR_PTR 0x04

// ?溢出计数器?：当FIFO数据溢出时（旧数据被新数据覆盖），此计数器会递增。??（读写）?
#define REG_OVF_COUNTER 0x05

// ?FIFO读指针?：指向传感器内部FIFO数据缓冲区中下一个要读取的位置。??（读写）?
#define REG_FIFO_RD_PTR 0x06

// FIFO数据寄存器?：?这是最重要的寄存器！?? 你通过I2C连续读取这个寄存器，就可以获取到红光和红外光的原始ADC采样数据。??（只读）?
#define REG_FIFO_DATA 0x07

// 2.配置寄存器（如何工作）
// ?FIFO配置?：设置FIFO的采样平均数量、FIFO滚溢覆盖使能等。
#define REG_FIFO_CONFIG 0x08

// ?模式配置?：?核心寄存器！?? 设置芯片的工作模式（心率模式、血氧模式、多LED模式、关机等）。
#define REG_MODE_CONFIG 0x09

// ?血氧配置?：?核心寄存器！?? 设置血氧模式下的ADC精度（采样位数）和采样率（每秒采集多少次）
#define REG_SPO2_CONFIG 0x0A
// LED1（红光）脉冲幅度控制?：设置红光LED的发光亮度（电流大小）。值越大，亮度越高，功耗越大，信号强度也越强。
#define REG_LED1_PA 0x0C

// ?LED2（红外）脉冲幅度控制?：设置红外光LED的发光亮度。
#define REG_LED2_PA 0x0D
// Pilot LED脉冲幅度控制?：在多LED模式下，控制其他LED的亮度。
#define REG_PILOT_PA 0x10

// 3. 多LED控制寄存器
// ?多LED控制 1?：在 multi-LED 模式下，配置哪个时隙（time slot）由哪个LED发光。
#define REG_MULTI_LED_CTRL1 0x11

// ?多LED控制 2?：同上，用于扩展配置更多的时隙。
#define REG_MULTI_LED_CTRL2 0x12

// 4. 温度传感器寄存器
// 温度整数部分?：读取芯片内部温度传感器的整数部分。
#define REG_TEMP_INTR 0x1F

// ?温度小数部分?：读取温度值的小数部分。
#define REG_TEMP_FRAC 0x20

// ?温度配置?：启动一次温度转换。
#define REG_TEMP_CONFIG 0x21

// 5. 其他寄存器
// ?接近中断阈值?：设置一个阈值，当信号强度超过它时触发接近检测中断。
#define REG_PROX_INT_THRESH 0x30

// ?修订ID?：读取芯片的修订版本号。??（只读）?
#define REG_REV_ID 0xFE

// 部件ID?：?只读，用于验证芯片是否连接正常。MAX30102的该寄存器值应为 0x15
#define REG_PART_ID 0xFF

// MAX30102所有操作函数
void MAX30102_Init(void);
void MAX30102_Reset(void);
uint8_t M30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data);
uint8_t max30102_Bus_Read(uint8_t Register_Address);
void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count);
void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data);

void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data);
void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data);
void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led);

#endif

//好的，我们来详细讲解这些宏定义，它们对应了MAX30102心率血氧传感器内部各个控制寄存器的地址。你可以把这些寄存器理解为芯片内部的一系列开关、配置项和数据缓冲区，通过I2C总线读写这些寄存器就可以完全控制这颗芯片。

// 以下是每个宏定义对应的寄存器功能详解：

// 1. 基本控制与状态寄存器

// 宏定义 寄存器地址 功能说明

// REG_INTR_STATUS_1 0x00 中断状态 1：读取该寄存器可以查看哪些中断被触发了（例如：数据就绪、环境光溢出等）。（只读）

// REG_INTR_STATUS_2 0x01 中断状态 2：查看其他中断状态，比如温度读数就绪等。（只读）

// REG_INTR_ENABLE_1 0x02 中断使能 1：向该寄存器写入特定的位模式，可以启用或禁用REG_INTR_STATUS_1中对应的中断源。（读写）

// REG_INTR_ENABLE_2 0x03 中断使能 2：用于启用或禁用REG_INTR_STATUS_2中的中断源。（读写）

// REG_FIFO_WR_PTR 0x04 FIFO写指针：指向传感器内部FIFO数据缓冲区中下一个要写入的位置。（读写）

// REG_OVF_COUNTER 0x05 溢出计数器：当FIFO数据溢出时（旧数据被新数据覆盖），此计数器会递增。（读写）

// REG_FIFO_RD_PTR 0x06 FIFO读指针：指向传感器内部FIFO数据缓冲区中下一个要读取的位置。（读写）

// REG_FIFO_DATA 0x07 FIFO数据寄存器：这是最重要的寄存器！ 你通过I2C连续读取这个寄存器，就可以获取到红光和红外光的原始ADC采样数据。（只读）
// 2. 配置寄存器（如何工作）
// 宏定义 寄存器地址 功能说明

// REG_FIFO_CONFIG 0x08 FIFO配置：设置FIFO的采样平均数量、FIFO滚溢覆盖使能等。

// REG_MODE_CONFIG 0x09 模式配置：核心寄存器！ 设置芯片的工作模式（心率模式、血氧模式、多LED模式、关机等）。

// REG_SPO2_CONFIG 0x0A 血氧配置：核心寄存器！ 设置血氧模式下的ADC精度（采样位数）和采样率（每秒采集多少次）。

// REG_LED1_PA 0x0C LED1（红光）脉冲幅度控制：设置红光LED的发光亮度（电流大小）。值越大，亮度越高，功耗越大，信号强度也越强。

// REG_LED2_PA 0x0D LED2（红外）脉冲幅度控制：设置红外光LED的发光亮度。

// REG_PILOT_PA 0x10 Pilot LED脉冲幅度控制：在多LED模式下，控制其他LED的亮度。
// 3. 多LED控制寄存器
// 宏定义 寄存器地址 功能说明

// REG_MULTI_LED_CTRL1 0x11 多LED控制 1：在 multi-LED 模式下，配置哪个时隙（time slot）由哪个LED发光。

// REG_MULTI_LED_CTRL2 0x12 多LED控制 2：同上，用于扩展配置更多的时隙。
// 4. 温度传感器寄存器
// 宏定义 寄存器地址 功能说明

// REG_TEMP_INTR 0x1F 温度整数部分：读取芯片内部温度传感器的整数部分。

// REG_TEMP_FRAC 0x20 温度小数部分：读取温度值的小数部分。

// REG_TEMP_CONFIG 0x21 温度配置：启动一次温度转换。
// 5. 其他寄存器
// 宏定义 寄存器地址 功能说明

// REG_PROX_INT_THRESH 0x30 接近中断阈值：设置一个阈值，当信号强度超过它时触发接近检测中断。

// REG_REV_ID 0xFE 修订ID：读取芯片的修订版本号。（只读）

// REG_PART_ID 0xFF 部件ID：只读，用于验证芯片是否连接正常。MAX30102的该寄存器值应为 0x15。
// 6. I2C地址宏定义
// 宏定义 值 功能说明

// max30102_WR_address 0xAE 芯片的I2C写地址（8位格式，包含R/W位）。0xAE = 0x57 << 1 | I2C_WR(0)

// I2C_WRITE_ADDR 0xAE 同上，I2C写地址。

// I2C_READ_ADDR 0xAF 芯片的I2C读地址（8位格式）。0xAF = 0x57 << 1 | I2C_RD(1)

// I2C_WR 0 I2C协议中的写控制位值（0）。

// I2C_RD 1 I2C协议中的读控制位值（1）。

// 总结与使用流程

// 这些宏定义为你提供了一个清晰的“控制面板”，初始化和使用MAX30102的典型流程如下：

// 1.  初始化I2C总线。
// 2.  读取REG_PART_ID 确认芯片连接正常。
// 3.  软件复位：通过向REG_MODE_CONFIG寄存器写入特定值让芯片复位。
// 4.  配置FIFO：设置REG_FIFO_CONFIG。
// 5.  设置采样率/精度：配置REG_SPO2_CONFIG。
// 6.  设置LED亮度：配置REG_LED1_PA和REG_LED2_PA。
// 7.  设置工作模式：配置REG_MODE_CONFIG，启动测量。
// 8.  启用中断：配置REG_INTR_ENABLE_1等（如果需要）。
// 9.  循环读取数据：等待中断或查询状态，然后连续读取REG_FIFO_DATA寄存器获取原始光数据。
// 10. 调用算法：将原始数据送入算法函数计算心率和血氧。

// 这些宏定义极大地提高了代码的可读性和可维护性，让你无需记忆复杂的十六进制地址，只需操作有意义的名称即可。
