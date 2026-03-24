// 这段代码是ATGM336H GPS模块的驱动和数据处理模块，同时支持GPS和北斗系统,主要功能是从GPS模块接收原始NMEA协议数据流，解析出经纬度坐标信息，并将其转换为可用的十进制格式
#include "atgm336h.h"

char longitude[64] = "123N"; // 经度
char latitude[64] = "456E";  // 纬度
// //串口中断 + 超时解析

//  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//      if (huart->Instance == USART2)
//      {
//          uart_rx_ticks = uwTick;
//          uart_rx_index++;
//          HAL_UART_Receive_IT(&huart2, &uart_rx_buffer[uart_rx_index], 1);
//      }
//  }

/*NMEA数据解析函数 atgm336h_process*/
// 这是GPS数据解析的核心函数，主要完成以下任务：
// 代码中并没有区分GPS和北斗数据，而是统一处理GNSS数据

// 在接收到的数据中查找NMEA协议的$GNGGA和$GNGLL标志
// 提取有效的GPS数据段
// 使用逗号分隔符解析NMEA字段
// 将GPS输出的度分格式转换为十进制格式
// 根据方向标志(S/W)确定正负值
void atgm336h_process(unsigned char *uartReadBuff)
{
  // 查找字符串 "$GNGGA" 和 "\r\n$GNGLL"，分别表示开始和结束的标志
  char *start = strstr((const char *)uartReadBuff, "$GNGGA");
  char *end = strstr((const char *)uartReadBuff, "\r\n$GNGLL");

  // 如果没有找到这两个标志，说明没有找到有效的 GPS 数据
  if (start == NULL || end == NULL)
  {
    // 如果需要调试，可以打印出来
    // printf("[GPS]$GNGGA line NOT found\r\n");
  }
  else
  {
    // 成功找到开始和结束标志，提取数据
    // 创建一个足够存储 $GNGGA 数据的缓冲区
    char gngga[100];

    // 从 uartReadBuff 中提取从 $GNGGA 到 $GNGLL 之间的数据
    strncpy(gngga, start, end - start);
    gngga[end - start] = '\0'; // 确保字符串以 NULL 结尾

    // 打印提取的 GNGGA 数据（调试用）
    printf("---%s\r\n", gngga);

    // 定义分隔符和一个数组来存储解析出的字段
    char *token;
    token = strtok(gngga, ","); // 使用逗号分隔每个字段
    char *nmea_fields[15];      // 最多支持 15 个字段
    int i = 0;

    // 逐个字段提取并存入 nmea_fields 数组中
    while (token != NULL)
    {
      nmea_fields[i] = token;
      token = strtok(NULL, ","); // 获取下一个字段
      i++;
      if (i >= 15)
        break; // 限制字段数为 15
    }

    // 如果提取到的字段大于 6，说明数据有效
    if (i > 6)
    {
      // 成功提取到数据，处理经纬度
      // printf("[GPS]data found\r\n");

      // 解析纬度
      int lat_deg = (int)(atof(nmea_fields[2]) / 100);         // 取出度数（例如：2056.122314 -> 20）
      double lat_min = atof(nmea_fields[2]) - (lat_deg * 100); // 取出分数（例如：2056.122314 - 20*100 = 56.122314）

      // 计算纬度
      float latitude_decimal = lat_deg + (lat_min / 60);
      if (nmea_fields[3][0] == 'S') // 如果是南纬，取负
        latitude_decimal = -latitude_decimal;

      // 解析经度
      int lon_deg = (int)(atof(nmea_fields[4]) / 100);        // 取出度数（例如：11002.398438 -> 110）
      float lon_min = atof(nmea_fields[4]) - (lon_deg * 100); // 取出分数（例如：11002.398438 - 110*100 = 2.398438）

      // 计算经度
      float longitude_decimal = lon_deg + (lon_min / 60);
      if (nmea_fields[5][0] == 'W') // 如果是西经，取负
        longitude_decimal = -longitude_decimal;
      // 打印转换后的经纬度数据（调试用）
      // printf("[GPS]Latitude: %.6f, Longitude: %.6f\r\n", latitude_decimal, longitude_decimal);
    }
    else
    {
      // 如果字段数不够，说明数据无效
      // printf("[GPS]data NOT found\r\n");
    }
  }
  // printf("---GPS_END---\r\n"); // 可选的调试输出
}

// void atgm336h_task(void)
// {
//     // 如果接收索引为0，说明没有数据需要处理，直接返回
//     if (uart_rx_index == 0)
//         return;
//     // 如果从最后一次接收到数据到现在已经超过100ms
//     if (uwTick - uart_rx_ticks > 10) // 100ms内没有收到新的数据
//     {
//         //printf("uart data: %s\n", uart_rx_buffer);
//         // 调用 GPS 解析函数，处理接收到的数据
//         atgm336h_process((unsigned char *)uart_rx_buffer);

//         // 清空接收缓冲区，将接收索引置零
//         memset(uart_rx_buffer, 0, uart_rx_index);
//         uart_rx_index = 0;

//         // 将UART接收缓冲区指针重置为接收缓冲区的起始位置
//         huart2.pRxBuffPtr = uart_rx_buffer;
//     }
// }

// DMA + 空闲中断 atgm336h_process()函数不变 不需要注释
#define BUUFER_SIZE 1000

// 定义环形缓冲区和接收缓冲区
ringbuffer_t usart_rb;
uint8_t usart_read_buffer[BUUFER_SIZE];

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  //       printf("dma data:%s\r\n", uart_rx_dma_buffer);
  // 如果环形缓冲区未满
  if (!ringbuffer_is_full(&usart_rb))
  {
    // 将DMA缓冲区中的数据写入环形缓冲区
    ringbuffer_write(&usart_rb, uart_rx_dma_buffer, Size);
  }
  // 清空DMA缓冲区
  memset(uart_rx_dma_buffer, 0, sizeof(uart_rx_dma_buffer));
}

// 主任务函数
// 检查环形缓冲区中是否有新数据
// 从环形缓冲区读取数据
// 调用atgm336h_process函数解析数据
// 清空处理缓冲区
void atgm336h_task(void)
{
  // 如果环形缓冲区为空，直接返回
  if (ringbuffer_is_empty(&usart_rb))
    return;
  // 从环形缓冲区读取数据到读取缓冲区
  ringbuffer_read(&usart_rb, usart_read_buffer, usart_rb.itemCount);

  //       // 打印读取缓冲区中的数据
  //      printf("ringbuffer data: %s\n", usart_read_buffer);
  // 调用 GPS 解析函数，处理接收到的数据
  atgm336h_process((unsigned char *)usart_read_buffer);

  // 清空读取缓冲区
  memset(usart_read_buffer, 0, sizeof(uint8_t) * BUUFER_SIZE);
}

// 这段代码是 ?ATGM336H GPS模块的驱动和数据处理模块。它的核心任务是从GPS模块接收到的原始数据流中，解析出精确的经纬度坐标。

// 这段代码是干什么的？（核心功能）
// 它完成了从“GPS模块发出的原始文本数据”到“可用的经纬度数值”的转换。其工作流程可以清晰地通过以下流程图展示：
// flowchart TD
//     A[GPS模块持续输出<br>NMEA协议数据流] --> B{DMA接收<br>+ 空闲中断}
//     B -- 数据就绪 --> C[触发回调函数<br>将数据存入环形缓冲区]
//     C --> D[主循环调用atgm336h_task]
//     D --> E[从环形缓冲区读取数据块]
//     E --> F{调用atgm336h_process<br>查找$GNGGA和$GNGLL标志}
//     F -- 找到有效数据段 --> G[分割字段, 提取经纬度原始字符串]
//     G --> H[转换度分格式为十进制度数]
//     H --> I[存储或输出最终经纬度]
//     F -- 未找到有效标志 --> J[丢弃数据, 等待下次接收]
// 它具体有什么用？（应用价值）
// 这段代码的价值在于它将GPS模块输出的复杂文本协议，转换成了程序可以轻松使用的数值：

// ?提供位置信息?：这是最核心的功能。通过解析，你能得到设备当前所在的经度和纬度，这是所有定位、导航、轨迹记录功能的基础。

// ?处理异步数据流?：GPS模块会不停地通过串口发送数据。这段代码利用DMA（直接存储器访问）和空闲中断来高效地接收这些数据，而不是频繁地打断CPU（像注释掉的普通串口中断方式那样）。这种方式大大降低了CPU开销。

// ?协议解析?：GPS模块遵循NMEA-0183标准协议，数据是纯文本的，像 $GNGGA,092204.999,4250.5589,S,14718.5084,E,1,03,24.4,19.7,M,,,0000 * 1F。这段代码能从中精准地找到需要的那一段（$GNGGA），并从中提取出关键信息。

// ?数据格式转换?：GPS输出的经纬度格式是“度度分分.分分分分”（如 4250.5589,S表示南纬42度50.5589分）。这段代码将其转换为我们更常用的十进制格式?（如 -42.842648），方便在地图API或数学计算中使用。

// ?数据缓冲与解耦?：使用环形缓冲区（ring buffer）?? 是一个高级且实用的技巧。它让数据接收（在中断中发生）和数据处理（在主循环中发生）?异步进行，互不干扰和阻塞，提高了系统的稳定性和可靠性。

// 关键组成部分解析
// ?atgm336h_process函数?：?核心解析器。它接收一块数据，查找特定的句子起始标志（$GNGGA）和结束标志（\r\n$GNGLL），确保抓取到一段完整的GPS数据。然后用 strtok函数以逗号为分隔符，将句子分割成多个字段（如时间、纬度、纬度方向、经度、经度方向等），最后进行坐标换算。

// ?HAL_UARTEx_RxEventCallback函数?：?DMA空闲中断回调函数。当串口接收完一帧数据（检测到总线空闲）后，会自动调用此函数。它负责将DMA接收缓冲区中的数据搬运到环形缓冲区中，并清空DMA缓冲区以准备下一次接收。这是高效接收的关键。

// ?atgm336h_task函数?：?主任务函数。需要在主循环中定期调用。它检查环形缓冲区中是否有数据，如果有就读取出来，并调用 atgm336h_process进行解析。

// ?环形缓冲区（ringbuffer_t usart_rb）??：一个先进先出（FIFO）?? 的队列，是连接中断接收和主循环处理的桥梁，解决了双方速度不匹配的问题。
