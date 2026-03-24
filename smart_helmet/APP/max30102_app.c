#include "max30102_app.h"

// 定义一个标志位
bool measure_flag = 0; // 模块运行标志位

bool heartrate_flag = 0; // 心率异常标志位
bool spo2_flag = 0;      // 血氧异常标志位

// 初始化数据结构
MAX30102_Data max30102_data = {
    .buffer_length = BUFFER_LENGTH, // 缓冲区长度
    .min_value = 0x3FFFF,           // 初始最小值
    .max_value = 0,                 // 初始最大值
    .brightness = 0                 // 初始亮度值
};

/********************************** 滤波算法的全局变量 *************************************************/

// 滑动平均滤波器缓存
int hr_buffer[WINDOW_SIZE] = {0};   // 用于心率的滑动窗口
int spo2_buffer[WINDOW_SIZE] = {0}; // 用于血氧的滑动窗口
int hr_index = 0, spo2_index = 0;   // 缓存索引

// 低通滤波器的先前值
int prev_hr = 0, prev_spo2 = 0; // 上一时刻的平滑心率和血氧值

/********************************** 滤波算法的全局变量 *************************************************/

/********************************** 滤波算法 *************************************************/
// 滑动平均滤波函数
// 工作原理:
// 维护一个固定大小的数据窗口 (WINDOW_SIZE)
// 每次有新数据时，替换窗口中最旧的数据
// 计算窗口内所有数据的平均值作为滤波结果
// 使用循环缓冲区技术实现高效的数据更新
int SmoothData(int new_value, int *buffer, int *index)
{
  buffer[*index] = new_value;          // 更新滑动窗口
  *index = (*index + 1) % WINDOW_SIZE; // 循环索引

  int sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++)
  {
    sum += buffer[i]; // 计算窗口内的平均值
  }

  return sum / WINDOW_SIZE; // 返回平均值
}

// 低通滤波函数
// 工作原理:
// 使用一阶IIR(无限脉冲响应)滤波器
// 通过系数ALPHA控制滤波强度
// 当前输出 = α × 当前输入 + (1-α) × 上一次输出
// 这是一种简单的指数平滑滤波器
int LowPassFilter(int new_value, int previous_filtered_value)
{
  return (int)(ALPHA * new_value + (1 - ALPHA) * previous_filtered_value); // 按公式进行滤波
}

/********************************** 滤波算法 *************************************************/

// 读取MAX30102传感器数据，并进行心率补偿
void MAX30102_Read_Data(void)
{
  volatile uint32_t un_prev_data;
  uint8_t temp[6];
  // 读取前500个样本，确定信号范围
  for (int i = 0; i < max30102_data.buffer_length; i++)
  {
    while (MAX30102_INT == 1)
      ; // 等待中断信号

    max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp); // 从FIFO读取数据
    max30102_data.red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];
    max30102_data.ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

    // 更新信号的最小值和最大值
    if (max30102_data.min_value > max30102_data.red_buffer[i])
      max30102_data.min_value = max30102_data.red_buffer[i];
    if (max30102_data.max_value < max30102_data.red_buffer[i])
      max30102_data.max_value = max30102_data.red_buffer[i];
  }

  // 更新上一数据点
  un_prev_data = max30102_data.red_buffer[max30102_data.buffer_length - 1];

  // 调整补偿值：减少补偿并应用在心率计算前
  max30102_data.heart_rate += HEART_RATE_COMPENSATION; // 可减少补偿值
}

// 计算心率和血氧值
void Calculate_Heart_Rate_and_SpO2(void)
{
  maxim_heart_rate_and_oxygen_saturation(max30102_data.ir_buffer, max30102_data.buffer_length,
                                         max30102_data.red_buffer, &max30102_data.spO2, &max30102_data.spO2_valid,
                                         &max30102_data.heart_rate, &max30102_data.heart_rate_valid);
}

// 更新信号的最小值和最大值，并应用滤波
void Update_Signal_Min_Max(void)
{
  uint32_t un_prev_data = max30102_data.red_buffer[max30102_data.buffer_length - 1];

  for (int i = 100; i < max30102_data.buffer_length; i++)
  {
    // 移位数据
    max30102_data.red_buffer[i - 100] = max30102_data.red_buffer[i];
    max30102_data.ir_buffer[i - 100] = max30102_data.ir_buffer[i];

    // 低通滤波处理
    max30102_data.red_buffer[i - 100] = LowPassFilter(max30102_data.red_buffer[i - 100], un_prev_data);
    max30102_data.ir_buffer[i - 100] = LowPassFilter(max30102_data.ir_buffer[i - 100], un_prev_data);

    // 滑动平均滤波
    max30102_data.red_buffer[i - 100] = SmoothData(max30102_data.red_buffer[i - 100], hr_buffer, &hr_index);
    max30102_data.ir_buffer[i - 100] = SmoothData(max30102_data.ir_buffer[i - 100], spo2_buffer, &spo2_index);

    // 更新最小值和最大值
    if (max30102_data.min_value > max30102_data.red_buffer[i - 100])
      max30102_data.min_value = max30102_data.red_buffer[i - 100];
    if (max30102_data.max_value < max30102_data.red_buffer[i - 100])
      max30102_data.max_value = max30102_data.red_buffer[i - 100];
  }
}

// 显示相关字符串
uint8_t dis_hr = 0;   // 显示的心率值
uint8_t dis_spo2 = 0; // 显示的血氧值

// 数据处理与显示
void Process_And_Display_Data(void)
{
  if (max30102_data.heart_rate_valid == 1 && max30102_data.heart_rate < 120)
  {
    dis_hr = max30102_data.heart_rate;
    dis_spo2 = max30102_data.spO2;
    // printf("dis_hr:%d  ,dis_spo2:%d\r\n", dis_hr, dis_spo2);
  }
  //  else
  //  {
  //    dis_hr = 0;
  //    dis_spo2 = 0;
  //    printf("dis_hr:%d  ,dis_spo2:%d\r\n", dis_hr, dis_spo2);
  //  }
}

//// 旧的执行代码(整体功能实现之前)
// void max30102_task(void)
//{

//    MAX30102_Read_Data();
//    Calculate_Heart_Rate_and_SpO2();
//    Update_Signal_Min_Max();
//    Process_And_Display_Data();
//}

// 新的执行代码(整体功能实现之后)
void max30102_task(void)
{
  // 如果测量标志位为 0，直接返回
  if (measure_flag == 0)
    return;

  // 重置测量标志位 防止重复调用
  measure_flag = 0;

  // 循环直到获取有效的心率和血氧数据
  while (dis_hr == 0 && dis_spo2 == 0)
  {
    // 测量过程中需要关闭中断，避免干扰
    // 关闭中断可以2选一
    __disable_irq(); // 关闭所有中断
    // HAL_NVIC_DisableIRQ(USART2_IRQn); // 也可以只关闭ATGM336H定位模块对应的串口2的中断

    // MAX30102_Read_Data()：从传感器读取原始数据
    MAX30102_Read_Data();

    // Calculate_Heart_Rate_and_SpO2()：计算心率和血氧值
    Calculate_Heart_Rate_and_SpO2();

    // Update_Signal_Min_Max()：更新信号的最大最小值
    Update_Signal_Min_Max();

    // Process_And_Display_Data()：处理并显示数据
    Process_And_Display_Data();
  }
  // 检测完出来之后再把中断打开 __enable_irq()和HAL_NVIC_EnableIRQ(USART2_IRQn)必须都开 不能只开一个
  // __enable_irq()相当于总闸 HAL_NVIC_EnableIRQ(USART2_IRQn)相当于单个外设的开关
  __enable_irq();                  // 开启所有中断
  HAL_NVIC_EnableIRQ(USART2_IRQn); // 开启ATGM336H定位模块对应的串口2的中断

  // 心率异常判断：正常范围60-100次/分钟
  heartrate_flag = (dis_hr < 60 || dis_hr > 100);

  // printf("heartrate_flag=%d\r\n", heartrate_flag);

  // 血氧异常判断：正常范围95%-100%
  spo2_flag = (dis_spo2 < 95 || dis_spo2 > 100);

  // printf("spo2_flag=%d\r\n", spo2_flag);
}

// 这段代码是MAX30102心率血氧传感器的应用层驱动和数据处理模块。它负责控制传感器、读取原始数据、调用算法进行计算，并对结果进行后处理和状态判断。

// 这段代码是干什么的？（核心功能）
// 简单来说，它扮演了?“设备经理”?和?“数据分析师”?的双重角色，其工作流程可以清晰地通过以下序列图展示：
// sequenceDiagram
//     participant 用户应用
//     participant max30102_task
//     participant MAX30102传感器
//     participant 核心算法库
//     participant 滤波模块

//     用户应用 ->> max30102_task: 设置 measure_flag = 1
//     loop 直到获取有效数据
//         max30102_task ->> MAX30102传感器: MAX30102_Read_Data()
//         MAX30102传感器 -->> max30102_task: 返回原始光信号数据
//         max30102_task ->> 核心算法库: Calculate_Heart_Rate_and_SpO2()
//         核心算法库 -->> max30102_task: 返回初步心率和血氧值
//         max30102_task ->> 滤波模块: Update_Signal_Min_Max()
//         滤波模块 -->> max30102_task: 返回滤波后的平滑数据
//         max30102_task ->> max30102_task: Process_And_Display_Data()
//     end
//     max30102_task ->> max30102_task: 判断心率/血氧是否异常
//     max30102_task -->> 用户应用: 更新 heartrate_flag, spo2_flag
// 它具体有什么用？（应用价值）
// 这段代码的价值在于它将复杂的底层操作封装成了简单可靠的应用功能：

// ?提供简洁的应用接口?：主程序（如main函数中的循环）只需要定期检查measure_flag并调用max30102_task()函数，就能获取到最新的心率和血氧数据，无需关心复杂的传感器控制和数据处理细节。

// ?实现数据后处理与优化?：在调用官方算法库之后，它还加入了额外的滤波处理?（滑动平均、低通滤波），使得最终显示的数据更加平滑、稳定，避免了数值的剧烈跳动，提升了用户体验。

// ?实现健康状态判断?：它不仅仅是测量，还定义了心率异常和血氧异常的判断标准（如心率<60或>100，血氧<95），并设置了相应的标志位（heartrate_flag, spo2_flag）。这使得主程序可以非常方便地根据这些标志位来触发警报、提示或其他操作。

// ?管理传感器工作流程?：它控制了整个测量流程，包括等待传感器中断信号、读取FIFO数据、调用算法、数据处理等一系列步骤，确保了测量的有序性和正确性。

// ?资源与中断管理?：在max30102_task函数中，?在关键时刻关闭中断的操作确保了数据读取和计算的原子性，防止了其他任务（如串口通信）打断传感器数据的读取过程，从而避免了数据错乱，提高了测量的准确性。

// 关键组成部分解析
// ?全局变量与标志位?：

// measure_flag：任务触发标志。主程序设置此标志位为1后，max30102_task函数才会执行一次测量。

// heartrate_flag, spo2_flag：输出标志。告知主程序当前心率或血氧是否处于异常状态。

// max30102_data：数据结构体，用于存储原始数据、算法计算结果和信号特征信息。

// ?核心函数?：

// MAX30102_Read_Data()：?读取原始数据。等待传感器中断，然后从FIFO中读取红光和红外光的原始数据。

// Calculate_Heart_Rate_and_SpO2()：?调用算法。调用官方提供的maxim_heart_rate_and_oxygen_saturation函数，传入原始数据，计算出初步的心率和血氧值。

// Update_Signal_Min_Max()：?滤波与处理。对数据进行移位，并应用滑动平均和低通滤波，使信号更平滑。

// Process_And_Display_Data()：?结果处理。对算法计算的结果进行有效性判断和取舍，准备用于显示。

// max30102_task()：?主任务函数。是整个模块的入口，协调调用各个子函数，完成整个测量、计算、判断的流程。

// 总结
// 这段代码是一个承上启下的应用层模块。它对下（传感器硬件和底层算法）进行了封装和优化，对上（用户主程序）提供了简洁的接口和明确的状态标志。它的存在使得集成MAX30102传感器变得非常容易，开发者可以更专注于业务逻辑（例如：根据异常标志位让OLED屏幕显示警告，或者通过蓝牙发送数据），而不必深究复杂的信号处理细节。
