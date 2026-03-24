// 1. 代码服务对象
// 这段代码是为**MPU系列惯性测量单元(IMU)**服务的驱动程序头文件，主要支持：

// MPU6050：六轴传感器（3轴加速度计+3轴陀螺仪）
// MPU6500：升级版六轴传感器
// MPU9150/MPU9250：九轴传感器（六轴+3轴磁力计）
// 在您的智能安全头盔项目中，主要用于实现头盔的姿态检测和运动识别功能。

// 2. 主要用途
// 2.1 核心功能
// 传感器初始化：mpu_init()等函数用于初始化和配置传感器
// 数据读取：获取加速度、角速度、温度等原始数据
// DMP功能：通过硬件数字运动处理器进行姿态解算
// 中断管理：处理数据就绪、运动检测等中断
// 2.2 在头盔项目中的具体应用
// 姿态检测：通过mpu_dmp_get_data()获取头盔的俯仰角、横滚角和航向角
// 运动识别：检测摔倒、碰撞等异常状态
// 数据融合：利用DMP实现多传感器数据融合，提高测量精度
// 3. 学习建议
// 3.1 不推荐初学者直接学习的原因
// 复杂度高

// 涉及底层I2C通信协议
// 包含复杂的姿态解算算法
// 需要理解传感器工作原理
// 依赖性强

// 需要配合inv_mpu.c实现文件使用
// 依赖DMP固件
// 需要配套硬件支持
// 学习曲线陡峭

// 需要掌握C语言高级特性
// 需要了解嵌入式系统开发
// 需要具备一定的数学基础
// 3.2 建议的学习路径
// 基础阶段

// c
//  复制
//  插入
//  新文件

// // 先学习简单的传感器使用
// // 例如：读取加速度数据
// short ax, ay, az;
// MPU_Get_Accelerometer(&ax, &ay, &az);
// 进阶阶段

// 学习基本的姿态解算算法（如互补滤波）
// 理解加速度计和陀螺仪的数据融合
// 实践简单的姿态检测功能
// 高级阶段

// 学习四元数和欧拉角转换
// 理解DMP工作原理
// 实践复杂的运动识别算法
// 4. 总结
// 这段代码是一个功能完整的MPU系列传感器驱动程序，对于初学者来说过于复杂。建议：

// 先从基础的传感器使用开始
// 逐步学习简单的姿态解算算法
// 在掌握基础后再学习DMP等高级功能
// 在实际项目中，可以直接使用这些API，重点放在应用层开发上
// 这样的学习路径更加合理，也更容易理解和掌握相关知识点。

/*
 $License:
    Copyright (C) 2011-2012 InvenSense Corporation, All Rights Reserved.
    See included License.txt for License information.
 $
 */
/**
 *  @addtogroup  DRIVERS Sensor Driver Layer
 *  @brief       Hardware drivers to communicate with sensors via I2C.
 *
 *  @{
 *      @file       inv_mpu.h
 *      @brief      An I2C-based driver for Invensense gyroscopes.
 *      @details    This driver currently works for the following devices:
 *                  MPU6050
 *                  MPU6500
 *                  MPU9150 (or MPU6050 w/ AK8975 on the auxiliary bus)
 *                  MPU9250 (or MPU6500 w/ AK8963 on the auxiliary bus)
 */

#ifndef _INV_MPU_H_
#define _INV_MPU_H_

#include "bsp_system.h"

//定义输出速度
#define DEFAULT_MPU_HZ  (100)		//100Hz

#define INV_X_GYRO      (0x40)
#define INV_Y_GYRO      (0x20)
#define INV_Z_GYRO      (0x10)
#define INV_XYZ_GYRO    (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO)
#define INV_XYZ_ACCEL   (0x08)
#define INV_XYZ_COMPASS (0x01)

struct int_param_s {
    void *arg;
};

#define MPU_INT_STATUS_DATA_READY       (0x0001)
#define MPU_INT_STATUS_DMP              (0x0002)
#define MPU_INT_STATUS_PLL_READY        (0x0004)
#define MPU_INT_STATUS_I2C_MST          (0x0008)
#define MPU_INT_STATUS_FIFO_OVERFLOW    (0x0010)
#define MPU_INT_STATUS_ZMOT             (0x0020)
#define MPU_INT_STATUS_MOT              (0x0040)
#define MPU_INT_STATUS_FREE_FALL        (0x0080)
#define MPU_INT_STATUS_DMP_0            (0x0100)
#define MPU_INT_STATUS_DMP_1            (0x0200)
#define MPU_INT_STATUS_DMP_2            (0x0400)
#define MPU_INT_STATUS_DMP_3            (0x0800)
#define MPU_INT_STATUS_DMP_4            (0x1000)
#define MPU_INT_STATUS_DMP_5            (0x2000)

/* Set up APIs */
int mpu_init(struct int_param_s *int_param);
int mpu_init_slave(void);
int mpu_set_bypass(unsigned char bypass_on);

/* Configuration APIs */
int mpu_lp_accel_mode(unsigned char rate);
int mpu_lp_motion_interrupt(unsigned short thresh, unsigned char time,
    unsigned char lpa_freq);
int mpu_set_int_level(unsigned char active_low);
int mpu_set_int_latched(unsigned char enable);

int mpu_set_dmp_state(unsigned char enable);
int mpu_get_dmp_state(unsigned char *enabled);

int mpu_get_lpf(unsigned short *lpf);
int mpu_set_lpf(unsigned short lpf);

int mpu_get_gyro_fsr(unsigned short *fsr);
int mpu_set_gyro_fsr(unsigned short fsr);

int mpu_get_accel_fsr(unsigned char *fsr);
int mpu_set_accel_fsr(unsigned char fsr);

int mpu_get_compass_fsr(unsigned short *fsr);

int mpu_get_gyro_sens(float *sens);
int mpu_get_accel_sens(unsigned short *sens);

int mpu_get_sample_rate(unsigned short *rate);
int mpu_set_sample_rate(unsigned short rate);
int mpu_get_compass_sample_rate(unsigned short *rate);
int mpu_set_compass_sample_rate(unsigned short rate);

int mpu_get_fifo_config(unsigned char *sensors);
int mpu_configure_fifo(unsigned char sensors);

int mpu_get_power_state(unsigned char *power_on);
int mpu_set_sensors(unsigned char sensors);

int mpu_set_accel_bias(const long *accel_bias);

/* Data getter/setter APIs */
int mpu_get_gyro_reg(short *data, unsigned long *timestamp);
int mpu_get_accel_reg(short *data, unsigned long *timestamp);
int mpu_get_compass_reg(short *data, unsigned long *timestamp);
int mpu_get_temperature(long *data, unsigned long *timestamp);

int mpu_get_int_status(short *status);
int mpu_read_fifo(short *gyro, short *accel, unsigned long *timestamp,
    unsigned char *sensors, unsigned char *more);
int mpu_read_fifo_stream(unsigned short length, unsigned char *data,
    unsigned char *more);
int mpu_reset_fifo(void);

int mpu_write_mem(unsigned short mem_addr, unsigned short length,
    unsigned char *data);
int mpu_read_mem(unsigned short mem_addr, unsigned short length,
    unsigned char *data);
int mpu_load_firmware(unsigned short length, const unsigned char *firmware,
    unsigned short start_addr, unsigned short sample_rate);

int mpu_reg_dump(void);
int mpu_read_reg(unsigned char reg, unsigned char *data);
int mpu_run_self_test(long *gyro, long *accel);
int mpu_register_tap_cb(void (*func)(unsigned char, unsigned char));

//自加

// 方向转换
unsigned short inv_row_2_scale(const signed char *row);
// MPU6050自测试
// 返回值:0,正常
//     其他,失败
uint8_t run_self_test(void);
// 陀螺仪方向控制
unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);
// mpu6050,dmp初始化
// 返回值:0,正常
//     其他,失败
uint8_t mpu_dmp_init(void);
// 得到dmp处理后的数据(注意,本函数需要比较多堆栈,局部变量有点多)
// pitch:俯仰角 精度:0.1°   范围:-90.0° <---> +90.0°
// roll:横滚角  精度:0.1°   范围:-180.0°<---> +180.0°
// yaw:航向角   精度:0.1°   范围:-180.0°<---> +180.0°
// 返回值:0,正常
//     其他,失败
uint8_t mpu_dmp_get_data(float *pitch, float *roll, float *yaw);;

#endif  /* #ifndef _INV_MPU_H_ */

