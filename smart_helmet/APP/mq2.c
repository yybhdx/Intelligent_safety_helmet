#include "mq2.h" //添加mq.h头文件

// 我们现在讲一下ADC最基本的写法
// 西风的ADC用的是DMA 当然DMA更好我们可以直接照搬(顶多改一下引脚)蓝桥杯嵌入式的ADC+DMA的方式

float adc_value = 0; // 定义一个接收ADC 里取出转换好的数字结果的数

float voltage = 0; // 定义电压值 电压的英文为voltage

uint32_t dma_buffer[30]; // ADC的DMA接收缓存，通道30个采样点 到时候再像西风那样平均一下 保证数据值的稳定性

float RS = 0;       // Rs 表示传感器在不同浓度气体中的电阻值 可由ADC传出的值测得
float RL = 5.1f;    // 未知 由CSDN里面的博主测得
float R0 = 35.904f; // 未知 由CSDN里面的博主测得
float ppm = 0;      // 气体浓度 最终要得出的结果
bool density_flag=0;// 标志位

//// 旧的执行代码(整体功能实现之前)
//// DMA读取 ADC1
//void mq2_task(void)
//{
//  // adc_value给voltage赋完值之后就清零 方便下一次调用
//  adc_value = 0;
//  for (uint8_t i = 0; i < 30; i++)
//  {
//    adc_value += dma_buffer[i];
//  }

//  // (adc_value / 30)是为了平均读取的数据的结果 保证数据值的稳定性
//  // 3.3f ：是 STM32F103RBT6 的 ADC 参考电压（Vref），通常就是 MCU 的工作电压
//  // 4096 ：表⽰STM32F103RBT6 的 ADC 是 12 位的。12位ADC的分辨率，即4096个不同的数字值。
//  // * 3.3f / 4096 ：这个计算将ADC的数字输出值转化为实际的输⼊电压值。
//  voltage = (float)(adc_value / 30.0f) / 4095 * 3.3f;

//  // 打印电压值 用于串口测试
////  printf("voltage=%.2f\r\n", voltage);

//  // 得到ADC的值voltage后 我们已知的值是RL的电压voltage 可以测得RS的电阻
//  // 在 MQ-2 模块里，ADC 量到的 AO 电压(voltage)，其实就是固定电阻 RL 两端的电压
//  // RS 与 RL 在串联回路中分VC这个5V的电压
//  // 所以 (5.0 - voltage) 是RS的电压, (voltage / RL)是回路中的电流 二者相除就是RS的电阻
//  RS = ((5.0f - voltage) / voltage) * RL;

//  // CSDN上得到公式：Rs/R0 = 11.5428*ppm^(-0.6549) (这是用matlab进行仿真得到的计算烟雾浓度的函数)
//  // 此时可以用C 语言数学库（math.h）的pow()函数计算次方，例:pow(2, 3) → 结果是 2^3 = 8,pow(9, 0.5) → 结果是 9^{0.5} = 3（开平方）
//  // 用ChatGpt拆解式子得到ppm的式子
//  ppm = pow((RS / (R0 * 11.5428)), -1.5278);

//  // 打印浓度值 用于串口测试
//  printf("ppm=%.2f\r\n", ppm);
//}

// 新的执行代码(整体功能实现之后)
void mq2_task(void)
{
    adc_value = 0;
    for (uint8_t i = 0; i < 30; i++)
    {
        adc_value += dma_buffer[i];
    }
    voltage = (float)(adc_value / 30.0f) / 4095 * 3.3f;
		
		  // 打印VRL的电压值
		printf("voltage=%.2f\r\n", voltage);
    RS = ((5.0f - voltage) / voltage) * RL;
    // 测烟雾：Rs / R0 = 11.5428 * ppm ^ (-0.6549)
    ppm = pow((RS / (R0 * 11.5428)), -1.5278);
		// 打印浓度值 用于串口测试
		printf("ppm=%.2f\r\n", ppm);
    density_flag=(ppm>100); // 当ppm > 100时说明值不正常 density_flag为1
		// 打印标志位 用于串口测试
//		printf("density_flag=%d\r\n", density_flag);
}

//// 常规读取
// void mq2_task(void)
//{
//	// 清零 ADC 原始值
//	adc_value = 0;
//   // 启动 ADC（模数转换器）
//   // 为什么我们要开启ADC??
//   // 因为:在 STM32 里，ADC 默认是停着的，不启动它就不会去采集电压。
//   // 因为我们用的是ADC1 所以第一个参数是指向 ADC1 句柄的指针
//   HAL_ADC_Start(&hadc1);

//  // 等待 ADC 转换完成
//  // 因为使用的是ADC1 所以第一个参数是指向 ADC1 句柄的指针
//  // 第二个参数是等待时间
//  // #define HAL_MAX_DELAY 0xFFFFFFFFU  它表示**“一直等下去，直到完成”，也就是无限等待**。期间属于阻塞状态 会占用cpu资源
//  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

//  //   HAL_ADC_GetState(&hadc1)
//  // •	这个函数返回 ADC 的当前状态，类型是 uint32_t。
//  // •	你可以把它想成 ADC 内部的 状态寄存器，里面有很多“开关位”表示 ADC 的各种状态。
//  // •	例子：
//  // o	EOC（End Of Conversion）位 = ADC 完成一次转换
//  // o	BUSY 位 = ADC 正在工作

//  //   ② HAL_IS_BIT_SET(reg, bit)
//  // •	它不是普通函数，是一个宏，定义类似这样：
//  // #define HAL_IS_BIT_SET(REG, BIT) (((REG) & (BIT)) != 0U)
//  // •	功能：检查某个寄存器的某一位是不是 1。
//  // •	用大白话说：“这个开关有没有打开？”

//  //   ③ HAL_ADC_STATE_REG_EOC
//  // •	这是 HAL 库定义的 一个标志位，表示 ADC 转换完成（End Of Conversion）。
//  // •	类型通常是 uint32_t，它只对应寄存器里的一位。

//  // 大白话理解就是：
//  // 去问 ADC 现在的状态寄存器 HAL_ADC_GetState(&hadc1) 里面有哪些开关位被置位了。
//  // 用 HAL_IS_BIT_SET 检查 “EOC 转换完成位” 有没有被置位。
//  // 如果是 1 → 表示 ADC 已经完成一次转换 → if 条件成立。
//  // 如果是 0 → ADC 还没完成 → if 条件不成立。

//  // 等价的大白话写法
//  // uint32_t state = HAL_ADC_GetState(&hadc1);
//  // if (state & HAL_ADC_STATE_REG_EOC) // EOC 位是1就成立
//  // {
//  //   // ADC 转换完成
//  // }

//  if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) // 如果成了 则表示ADC转换完成
//  {
//    // 接收得到的值
//    adc_value = HAL_ADC_GetValue(&hadc1);

//    // 3.3f ：是 STM32F103RBT6 的 ADC 参考电压（Vref），通常就是 MCU 的工作电压
//    // 4096 ：表⽰STM32F103RBT6 的 ADC 是 12 位的。12位ADC的分辨率，即4096个不同的数字值。
//    // * 3.3f / 4096 ：这个计算将ADC的数字输出值转化为实际的输⼊电压值。
//    voltage = (float)adc_value / 4096 * 3.3f;

//    // 打印电压值 用于串口测试
//    printf("voltage=%.2f\r\n", voltage);
//  }
//}

