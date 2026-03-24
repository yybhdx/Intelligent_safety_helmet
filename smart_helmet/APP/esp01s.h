#ifndef _ESP01S_H
#define _ESP01S_H

#include "bsp_system.h"

// Wi-Fi 连接参数
#define WIFI_SSID "jifei"   // Wi-Fi 网络名称
#define WIFI_PWD "12345678" // Wi-Fi 密码

// 华为云物联网平台 MQTT 配置参数
#define HUAWEI_MQTT_USERNAME "68ac150094a9a05c33830954_smart_helmet"                                          // MQTT 用户名
#define HUAWEI_MQTT_PASSWORD "90a583e3866012f747a92ac7fca9ab320bbb83fac271b45297fdecc8814abb5a"               // MQTT 密码
#define HUAWEI_MQTT_ClientID "68ac150094a9a05c33830954_smart_helmet_0_0_2025082508"                           // MQTT 客户端 ID
#define HUAWEI_MQTT_ADDRESS "52e4e17470.st1.iotda-device.cn-east-3.myhuaweicloud.com"                         // MQTT 服务器地址
#define HUAWEI_MQTT_PORT "1883"                                                                               // MQTT 端口号
#define HUAWEI_MQTT_PUBLISH_TOPIC "$oc/devices/67bec08580c3872914ade8c0_smart_helmet_1/sys/properties/report" // 订阅的主题Topic

void esp_init(void);

void esp_report(void);

#endif
