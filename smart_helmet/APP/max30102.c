#include "max30102.h"

/* 指定地址写 */
// 功能：向MAX30102的指定寄存器写入一个字节数据
// 实现步骤：
// 发送I2C起始信号
// 发送设备地址(写模式)
// 等待应答
// 发送寄存器地址
// 等待应答
// 发送要写入的数据
// 等待应答
// 发送停止信号
// 返回值：1表示成功，0表示失败
uint8_t max30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data)
{

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	MAX30102_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址 */
	MAX30102_IIC_Send_Byte(Register_Address);
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第5步：开始写入数据 */
	MAX30102_IIC_Send_Byte(Word_Data);

	/* 第6步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();
	return 1; /* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();
	return 0;
}

/*指定地址读*/
// 功能：从MAX30102的指定寄存器读取一个字节数据
// 实现步骤：
// 发送I2C起始信号
// 发送设备地址(写模式)
// 等待应答
// 发送寄存器地址
// 等待应答
// 重新发送起始信号
// 发送设备地址(读模式)
// 等待应答
// 读取数据并发送NACK
// 发送停止信号
// 返回值：读取到的数据
uint8_t max30102_Bus_Read(uint8_t Register_Address)
{
	uint8_t data;

	/* 第1步：发起I2C总线启动信号 */
	MAX30102_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址， */
	MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	MAX30102_IIC_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：读取数据 */
	{
		data = MAX30102_IIC_Read_Byte(0); /* 读1个字节 */

		MAX30102_IIC_NAck(); /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	}
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();
	return data; /* 执行成功 返回data值 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();
	return 0;
}

/*FIFO数据读取函数*/
// 功能：从FIFO中读取多个18位的数据(红光和红外光)
// 参数：
// Register_Address：起始寄存器地址
// Word_Data：存储读取数据的二维数组
// count：要读取的数据个数
// 实现特点：
// 每个数据由3个字节组成(18位有效数据)
// 数据存储为16位格式，高位在前
// 每次读取包含红光和红外光两个数据
void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count)
{
	uint8_t i = 0;
	uint8_t no = count;
	uint8_t data1, data2;
	/* 第1步：发起I2C总线启动信号 */
	MAX30102_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址， */
	MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	MAX30102_IIC_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：读取数据 */
	while (no)
	{
		data1 = MAX30102_IIC_Read_Byte(0);
		MAX30102_IIC_Ack();
		data2 = MAX30102_IIC_Read_Byte(0);
		MAX30102_IIC_Ack();
		Word_Data[i][0] = (((uint16_t)data1 << 8) | data2); //

		data1 = MAX30102_IIC_Read_Byte(0);
		MAX30102_IIC_Ack();
		data2 = MAX30102_IIC_Read_Byte(0);
		if (1 == no)
			MAX30102_IIC_NAck(); /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		else
			MAX30102_IIC_Ack();
		Word_Data[i][1] = (((uint16_t)data1 << 8) | data2);

		no--;
		i++;
	}
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();
}

/*读取FIFO中的字节数据*/
// 功能：从FIFO中读取6个字节的原始数据
// 参数：
// Register_Address：寄存器地址
// Data：存储读取数据的数组
// 实现特点：
// 先清除中断状态寄存器
// 读取6个字节的原始数据
void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data)
{
	max30102_Bus_Read(REG_INTR_STATUS_1);
	max30102_Bus_Read(REG_INTR_STATUS_2);

	/* 第1步：发起I2C总线启动信号 */
	MAX30102_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址， */
	MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	MAX30102_IIC_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	MAX30102_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (MAX30102_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：读取数据 */
	Data[0] = MAX30102_IIC_Read_Byte(1);
	Data[1] = MAX30102_IIC_Read_Byte(1);
	Data[2] = MAX30102_IIC_Read_Byte(1);
	Data[3] = MAX30102_IIC_Read_Byte(1);
	Data[4] = MAX30102_IIC_Read_Byte(1);
	Data[5] = MAX30102_IIC_Read_Byte(0);
	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	MAX30102_IIC_Stop();

	//	uint8_t i;
	//	uint8_t fifo_wr_ptr;
	//	uint8_t firo_rd_ptr;
	//	uint8_t number_tp_read;
	//	//Get the FIFO_WR_PTR
	//	fifo_wr_ptr = max30102_Bus_Read(REG_FIFO_WR_PTR);
	//	//Get the FIFO_RD_PTR
	//	firo_rd_ptr = max30102_Bus_Read(REG_FIFO_RD_PTR);
	//
	//	number_tp_read = fifo_wr_ptr - firo_rd_ptr;
	//
	//	//for(i=0;i<number_tp_read;i++){
	//	if(number_tp_read>0){
	//		MAX30102_IIC_ReadBytes(max30102_WR_address,REG_FIFO_DATA,Data,6);
	//	}

	// max30102_Bus_Write(REG_FIFO_RD_PTR,fifo_wr_ptr);
}

/*复位传感器*/
// 功能：复位MAX30102传感器
// 实现：向MODE_CONFIG寄存器写入0x40两次
void MAX30102_Reset(void)
{
	max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
	max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
}

/*初始化传感器*/
// ?工作模式?：SpO2模式（交替点亮红光和红外光）。
// ?数据产出?：100Hz采样率，400μs脉冲宽度，ADC范围4096nA。
// ?数据管理?：FIFO满17个样本后触发中断，允许MCU来读取数据。
// ?中断方式?：使能了数据就绪和FIFO满中断，等待MCU响应。
// ?信号强度?：红光和红外LED均以约7mA的电流工作。
// 初始化完成后，传感器就会开始自动采集数据并将其填入FIFO。开发者只需要在MCU端等待中断，然后在中断服务程序中读取 REG_FIFO_DATA寄存器，即可获得原始的红光和红外光数据，供后续的心率、血氧算法使用。
/**
 * @brief  MAX30102传感器的初始化函数
 * @param  无
 * @retval 无
 */
/**
 * @brief MAX30102传感器初始化函数
 * 该函数用于初始化MAX30102传感器，包括GPIO配置、I2C通信初始化、传感器复位
 * 以及设置各种工作模式和参数
 */
void MAX30102_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO初始化结构体变量，用于配置GPIO引脚

	// 使能GPIOB时钟，这是使用GPIOB引脚的前提条件
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// 配置MAX30102中断引脚为输入模式，用于检测传感器产生的中断信号
	// 目的?：将连接MAX30102 ?INT（中断输出）?? 的STM32引脚配置为输入模式。这样MCU就可以检测到传感器何时有数据准备好，从而可以通过中断而非轮询的方式高效地读取数据。
	GPIO_InitStructure.Pin = MAX30102_INT_PIN;						 // 设置引脚号
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;						 // 设置为输入模式
	HAL_GPIO_Init(MAX30102_INT_PORT, &GPIO_InitStructure); // 初始化GPIO引脚

	// 初始化I2C通信，用于与MAX30102传感器进行数据传输
	// ?目的?：初始化I2C外设（如I2C1）。配置SCL和SDA引脚、设置I2C通信的速率（如400kHz）、模式等。这是与传感器进行通信的基础。
	MAX30102_IIC_Init();

	// 复位MAX30102传感器，恢复到初始状态
	// 目的?：通过向模式配置寄存器（REG_MODE_CONFIG, 0x09）?? 的bit7写入1，让传感器执行一次软件复位。这会使其所有寄存器恢复为默认值，确保从一个已知的状态开始配置。
	MAX30102_Reset();

	// 配置中断使能寄存器1（0x02）
	// ?值 0xC0(二进制 1100 0000)??：这意味着：
	// ?bit7 (A_FULL_EN) = 1?：使能 ?FIFO几乎满? 中断。当FIFO中的数据量达到设定的阈值时，传感器会拉低INT引脚通知MCU。
	// ?bit6 (PPG_RDY_EN) = 1?：使能 ?数据就绪? 中断。每当有一个新的采样数据被放入FIFO时，传感器都会触发中断。
	// 其他位为0，禁用其他中断源。
	max30102_Bus_Write(REG_INTR_ENABLE_1, 0xc0);

	// 配置中断使能寄存器2（0x03）?
	// 值 0x00?：禁用所有与此寄存器相关的中断（如温度测量就绪中断
	max30102_Bus_Write(REG_INTR_ENABLE_2, 0x00);

	// ?目的?：清零FIFO的写指针、溢出计数器和读指针。相当于清空传感器的数据缓冲区，从起点开始存储和读取数据。
	max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00); // FIFO_WR_PTR[4:0]
	max30102_Bus_Write(REG_OVF_COUNTER, 0x00); // OVF_COUNTER[4:0]
	max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00); // FIFO_RD_PTR[4:0]

	// 配置 ?FIFO配置寄存器（0x08）?
	// 	?值 0x0F?：
	// ?Bit [7:5] (SMP_AVE) = 000?：?采样平均数为1。即不对样本进行硬件平均，每个样本都单独输出。这提供了最高的时间分辨率。
	// ?Bit 4 (FIFO_ROLLOVER_EN) = 0?：?FIFO滚溢禁用。当FIFO满后，新的数据会覆盖旧的数据。
	// ?Bit [3:0] (FIFO_A_FULL) = 1111?：?FIFO几乎满阈值为17个样本。当FIFO中存储了17个样本（每个样本包含3字节红光 + 3字节红外光数据）时，会触发A_FULL中断。
	max30102_Bus_Write(REG_FIFO_CONFIG, 0x0f);

	// 配置最重要的模式配置寄存器（0x09）?，决定传感器的工作模式
	// ?值 0x03?：
	// ?Bit [2:0] (MODE) = 011?：将传感器设置为 ?SpO2模式。
	// 在此模式下，传感器会交替点亮红色LED和红外LED，并分别采集它们的透射光强度。这是计算心率和血氧饱和度所必需的模式。
	max30102_Bus_Write(REG_MODE_CONFIG, 0x03);

	// 	配置 ?血氧配置寄存器（0x0A）?，决定SpO2模式下的采样精度和速度。
	// 	值 0x27(二进制 0010 0111)??：
	// ?Bit [6:5] (SPO2_ADC_RGE) = 01?：设置ADC量程为4096nA。这决定了ADC的满量程范围。
	// ?Bit [4:2] (SPO2_SR) = 001?：设置采样率为100Hz?（每秒采集100个样本）。这对于捕捉心率信号是足够的。
	// ?Bit [1:0] (LED_PW) = 11?：设置LED脉冲宽度（积分时间）为400μs。更宽的脉冲宽度意味着更多的光被接收，信噪比更好，但功耗更高且更容易在运动时产生误差。
	max30102_Bus_Write(REG_SPO2_CONFIG, 0x27);

	// 分别配置红色LED（0x0C）?? 和红外LED（0x0D）?? 的发光亮度
	// 值 0x24?：这是一个经验值，对应于大约 ?7mA 的驱动电流。电流越大，信号越强，但电池消耗也越快。需要根据实际应用（如佩戴松紧度）进行调整。
	max30102_Bus_Write(REG_LED1_PA, 0x24);
	max30102_Bus_Write(REG_LED2_PA, 0x24);

	// 配置 ?Pilot LED电流（0x10）?
	// 注意?：此寄存器仅在多LED模式（Multi-LED Mode）?? 下生效。
	// 在当前的SpO2模式下，这个配置通常不会起作用。设置为 0x7F（最大电流）可能是一个保守的默认值。
	max30102_Bus_Write(REG_PILOT_PA, 0x7f);
}

/*写寄存器*/
// 功能：封装的寄存器写入函数
// 实现：调用MAX30102_IIC_Write_One_Byte函数
void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
	//  char ach_i2c_data[2];
	//  ach_i2c_data[0]=uch_addr;
	//  ach_i2c_data[1]=uch_data;
	//
	//  MAX30102_IIC_WriteBytes(I2C_WRITE_ADDR, ach_i2c_data, 2);
	MAX30102_IIC_Write_One_Byte(I2C_WRITE_ADDR, uch_addr, uch_data);
}

/*读寄存器*/
// 功能：封装的寄存器读取函数
// 实现：调用MAX30102_IIC_Read_One_Byte函数
void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
{
	//  char ch_i2c_data;
	//  ch_i2c_data=uch_addr;
	//  MAX30102_IIC_WriteBytes(I2C_WRITE_ADDR, &ch_i2c_data, 1);
	//
	//  i2c.read(I2C_READ_ADDR, &ch_i2c_data, 1);
	//
	//   *puch_data=(uint8_t) ch_i2c_data;
	MAX30102_IIC_Read_One_Byte(I2C_WRITE_ADDR, uch_addr, puch_data);
}

/*读取FIFO数据*/
// 功能：从FIFO中读取红光和红外光数据
// 参数：
// pun_red_led：红光数据指针
// pun_ir_led：红外光数据指针
// 实现特点：
// 清除中断状态
// 读取6字节的FIFO数据
// 组合成32位数据（实际使用18位）
// 使用掩码保留有效位（0x03FFFF）
void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
{
	uint32_t un_temp;
	unsigned char uch_temp;
	char ach_i2c_data[6];
	*pun_red_led = 0;
	*pun_ir_led = 0;

	// read and clear status register
	maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
	maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);

	MAX30102_IIC_ReadBytes(I2C_WRITE_ADDR, REG_FIFO_DATA, (uint8_t *)ach_i2c_data, 6);

	un_temp = (unsigned char)ach_i2c_data[0];
	un_temp <<= 16;
	*pun_red_led += un_temp;
	un_temp = (unsigned char)ach_i2c_data[1];
	un_temp <<= 8;
	*pun_red_led += un_temp;
	un_temp = (unsigned char)ach_i2c_data[2];
	*pun_red_led += un_temp;

	un_temp = (unsigned char)ach_i2c_data[3];
	un_temp <<= 16;
	*pun_ir_led += un_temp;
	un_temp = (unsigned char)ach_i2c_data[4];
	un_temp <<= 8;
	*pun_ir_led += un_temp;
	un_temp = (unsigned char)ach_i2c_data[5];
	*pun_ir_led += un_temp;
	*pun_red_led &= 0x03FFFF; // Mask MSB [23:18]
	*pun_ir_led &= 0x03FFFF;	// Mask MSB [23:18]
}
