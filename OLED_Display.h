#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H


#include "mbed.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 可配置参数
#define OLED_I2C_ADDR    0x78        // I2C的7位地址，注意匹配即可
#define OLED_W           128         // 屏幕宽度
#define OLED_H           64          // 屏幕高度
#define OLED_PAGES       (OLED_H/8)  // 页数量，8个像素1页

// 这个版本是带显存的，后续可以开发图像的显示，所以这里参数设为0
#ifndef OLED_AUTO_REFRESH
#define OLED_AUTO_REFRESH 0
#endif

// SSD1306 常用命令
#define OLED_CMD_OFF         0xAE   // 关闭显示 
#define OLED_CMD_ON          0xAF   // 开启显示 
#define OLED_CMD_CONTRAST    0x81   // 设置对比度（后跟 1 字节参数） 
#define OLED_CMD_START_LINE  0x40   // 设置显示起始行（0x40~0x7F） 
#define OLED_CMD_SEG_REMAP   0xA0   // 段重映射（|1 => 0xA1 左右镜像） 
#define OLED_CMD_DISP_NORM   0xA6   // 正常显示（|1 => 0xA7 反色） 
#define OLED_CMD_MUX         0xA8   // 多路复用比（后跟 1 字节参数） 
#define OLED_CMD_COM_SCAN    0xC0   // COM 扫描方向（|8 => 0xC8 上下镜像） 
#define OLED_CMD_OFFSET      0xD3   // 显示偏移（后跟 1 字节参数） 
#define OLED_CMD_CLOCK       0xD5   // 显示时钟分频/振荡频率（后跟参数） 
#define OLED_CMD_PRECHARGE   0xD9   // 预充电周期（后跟参数） 
#define OLED_CMD_COM_HW      0xDA   // COM 引脚硬件配置（后跟参数）
#define OLED_CMD_VCOMH       0xDB   // VCOMH 电压等级（后跟参数） 
#define OLED_CMD_CHARGE_PUMP 0x8D   // 电荷泵设置（后跟参数）

// 底层函数：写命令
void OLED_WR_CMD(uint8_t cmd);    

// 初始化
void OLED_Init(void);               

// 将显存推送到屏幕
void OLED_Refresh(void);            

// 清屏
void OLED_Clear(void);            

// 清行
void OLED_ClearLine(uint8_t line); 

// 调整对比度
void OLED_SetContrast(uint8_t val);    

// 反色
void OLED_InvertDisplay(uint8_t inv);   

// 单字符显示
void OLED_ShowChar(uint8_t x, uint8_t y, char chr);     

// 字符串显示
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

// 显存，帧缓冲
extern uint8_t OLED_GRAM[OLED_PAGES][OLED_W];

#ifdef __cplusplus
}
#endif

#endif 
