#include "mpu6050.h"

uint8_t i = 10;

// 姿态数据
float pitch, roll, yaw;    // 欧拉角
short aacx, aacy, aacz;    // 加速度传感器原始数据
short gyrox, gyroy, gyroz; // 陀螺仪原始数据

// 姿态数据
unsigned long walk;               // 步数计数
float steplength = 0.3, Distance; // 步距/米
uint8_t svm_set = 1;              // 路程
short GX, GY, GZ;                 // 陀螺仪数据

// 状态标志
bool fall_flag = 0;      // 跌倒标志
bool collision_flag = 0; // 碰撞标志
uint16_t AVM;            // 加速度矢量模长
uint16_t GVM;            // 角速度矢量模长

// 批量数据写
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    uint8_t data[1 + len]; // 第一个字节为寄存器地址，后面是要写入的数据
    data[0] = reg;
    for (uint8_t i = 0; i < len; i++)
        data[i + 1] = buf[i];

    if (HAL_I2C_Master_Transmit(&hi2c1, (addr << 1), data, len + 1, HAL_MAX_DELAY) != HAL_OK)
        return 1; // 传输失败
    return 0;     // 传输成功
}

// 批量数据读
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    // 发送寄存器地址
    if (HAL_I2C_Master_Transmit(&hi2c1, (addr << 1), &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return 1; // 传输寄存器地址失败

    // 读取数据
    if (HAL_I2C_Master_Receive(&hi2c1, (addr << 1), buf, len, HAL_MAX_DELAY) != HAL_OK)
        return 1; // 读取数据失败
    return 0;     // 读取成功
}

// 写入单字节数据
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    if (HAL_I2C_Master_Transmit(&hi2c1, (MPU_ADDR << 1), buf, 2, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1; // 传输失败
    }
    return 0; // 传输成功
}

// 读取单字节数据
uint8_t MPU_Read_Byte(uint8_t reg)
{
    uint8_t data;
    // 发送寄存器地址
    if (HAL_I2C_Master_Transmit(&hi2c1, (MPU_ADDR << 1), &reg, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // 传输失败
    }

    // 读取数据
    if (HAL_I2C_Master_Receive(&hi2c1, (MPU_ADDR << 1), &data, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // 读取失败
    }
    return data; // 读取成功
}

// 初始化MPU6050
void MPU_Init(void)
{
    //	MPU_IIC_Init(); //初始化IIC总线 main.c中的MX_I2C1_Init();已经帮我们全包括了 所以不需要了
    //  MX_I2C1_Init(); //初始化IIC总线 main.c中的MX_I2C1_Init();已经帮我们全包括了 所以不需要了
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x00);   // 解除休眠状态
    MPU_Write_Byte(MPU_SAMPLE_RATE_REG, 0x07); // 陀螺仪采样率，典型值：0x07(125Hz) // 设置采样率125Hz
    MPU_Write_Byte(MPU_CFG_REG, 0x06);         // 低通滤波频率，典型值：0x06(5Hz) // 设置低通滤波5Hz
    MPU_Write_Byte(MPU_GYRO_CFG_REG, 0x18);    // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
    MPU_Write_Byte(MPU_ACCEL_CFG_REG, 0x01);   // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
}

// 得到温度值
// 返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
    uint8_t buf[2];
    short raw;
    float temp;
    MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
    raw = ((uint16_t)buf[0] << 8) | buf[1];
    temp = 36.53 + ((double)raw) / 340; // 读取原始数据并转换为温度值
    return temp * 100;
}

// 得到陀螺仪值(原始值) //读的是角速度
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
uint8_t MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
    // 读取三轴角速度原始数据
    uint8_t buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
    if (res == 0)
    {
        *gx = ((uint16_t)buf[0] << 8) | buf[1];
        *gy = ((uint16_t)buf[2] << 8) | buf[3];
        *gz = ((uint16_t)buf[4] << 8) | buf[5];
    }
    return res;
    ;
}
// 得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
uint8_t MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
    // 读取三轴加速度原始数据

    uint8_t buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
    if (res == 0)
    {
        *ax = ((uint16_t)buf[0] << 8) | buf[1];
        *ay = ((uint16_t)buf[2] << 8) | buf[3];
        *az = ((uint16_t)buf[4] << 8) | buf[5];
    }
    return res;
    ;
}

//// 步数获取函数 可以判断你走了多少布 可以像微信那样记录你的步数
// void dmp_getwalk(void)
//{
//     dmp_get_pedometer_step_count(&walk);
//     printf("步数: %u\n", walk);  // 显示步数数值
//     Distance = steplength * walk;
//     printf("路程: %.2f cm\n", Distance);  // 显示路程，单位为 cm
// }

//// 旧的执行代码(整体功能实现之前)
////pitch:俯仰角 精度:0.1°   范围:-90.0° <---> +90.0°
////roll:横滚角  精度:0.1°   范围:-180.0°<---> +180.0°
////yaw:航向角   精度:0.1°   范围:-180.0°<---> +180.0°
// void mpu6050_task(void)
//{
//     mpu_dmp_get_data(&pitch, &roll, &yaw); // 获取数据 分别是pitch:俯仰角 roll:横滚角 yaw:航向角
//     printf("pitch:%0.1f   roll:%0.1f   yaw:%0.1f\r\n", pitch, roll, yaw); //打印角数据
// }

// 运动检测实现
// 新的执行代码(整体功能实现之后)
void mpu6050_task(void)
{
    // 数据融合
    // mpu_dmp_get_data()：通过DMP（Digital Motion Processor）获取处理后的姿态角（俯仰角pitch、横滚角roll、偏航角yaw）

    // 函数调用了MPU6050内置的DMP处理器进行数据融合，其特点：
    // 硬件级处理：融合算法在MPU6050芯片内部完成
    // 直接输出姿态角：无需额外的软件滤波算法
    // 高精度：使用预编译的固件进行数据融合
    mpu_dmp_get_data(&pitch, &roll, &yaw);

    // MPU_Get_Accelerometer()：获取原始三轴加速度数据（aacx, aacy, aacz）
    MPU_Get_Accelerometer(&aacx, &aacy, &aacz); // 得到加速度传感器数据

    // MPU_Get_Gyroscope()：获取原始三轴角速度数据（gyrox, gyroy, gyroz）
    MPU_Get_Gyroscope(&gyrox, &gyroy, &gyroz); // 得到dmp处理后的数据

    // 数据融合计算
    // AVM（Acceleration Vector Magnitude）：计算加速度矢量模长
    // GVM（Gyroscope Vector Magnitude）：计算角速度矢量模长
    AVM = sqrt(pow(aacx, 2) + pow(aacy, 2) + pow(aacz, 2));
    GVM = sqrt(pow(gyrox, 2) + pow(gyroy, 2) + pow(gyroz, 2)); // 计算角速度模长

    //    // 串口打印调试一下
    //    printf("AVM=%d\r\n", AVM);
    //    printf("GVM=%d\r\n", GVM);

    //  姿态判断
    // 判断是否跌倒（ pitch 或 roll 超过 60 度）
    // 跌倒检测（fall_flag）：
    // 姿态角判断：pitch或roll超过60度
    // 加速度判断：AVM小于10000或大于20000
    // 角速度判断：GVM大于100
    fall_flag = ((fabs(pitch) > 60 || fabs(roll) > 60) && (AVM < 10000 | AVM > 20000) && GVM > 100);

    // 碰撞检测（collision_flag）：仅通过加速度模长判断：AVM小于10000或大于20000
    // 判断是否碰撞 10000 < AVM 或 AVM > 20000 与 GVM > 100
    collision_flag = (AVM < 10000 | AVM > 20000);

    //		// 串口打印调试一下
    //    printf("fall_flag=%d\r\n", fall_flag);
    //    printf("collision_flag=%d\r\n", collision_flag);
}
