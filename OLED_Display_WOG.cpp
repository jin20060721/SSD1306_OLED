#include "OLED_Display_WOG.h"
#include "font.h"
#include "mbed.h"


extern I2C i2c;


static void OLED_SetPos(uint8_t page, uint8_t col) {
    OLED_WR_CMD(0xB0 | (page & 0x07));         // 页地址命令 0xB0~0xB7
    OLED_WR_CMD(0x00 | (col & 0x0F));          // 列地址低 4 位
    OLED_WR_CMD(0x10 | ((col >> 4) & 0x0F));   // 列地址高 4 位
}

void OLED_WR_CMD(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c.write(OLED_I2C_ADDR, (char *)buf, 2);
}

void OLED_WR_DATA(uint8_t dat) {
    uint8_t buf[2] = {0x40, dat};
    i2c.write(OLED_I2C_ADDR, (char *)buf, 2);
}


void OLED_Init(void) {
    thread_sleep_for(200);                 // 上电稳定等待

    OLED_WR_CMD(OLED_CMD_OFF);             // 0xAE 先关闭显示
    OLED_WR_CMD(0x00);                     // 列地址低 4 位 = 0
    OLED_WR_CMD(0x10);                     // 列地址高 4 位 = 0
    OLED_WR_CMD(OLED_CMD_START_LINE);      // 0x40 显示起始行 = 0
    OLED_WR_CMD(OLED_CMD_CONTRAST);        // 0x81 设置对比度
    OLED_WR_CMD(0xFF);                     //      对比度 = 255（最亮）
    OLED_WR_CMD(OLED_CMD_SEG_REMAP | 1);   // 0xA1 段重映射
    OLED_WR_CMD(OLED_CMD_DISP_NORM);       // 0xA6 正常显示
    OLED_WR_CMD(OLED_CMD_MUX);             // 0xA8 多路复用比
    OLED_WR_CMD(0x3F);                     //      0x3F => 64 行（1/64 duty）
    OLED_WR_CMD(OLED_CMD_COM_SCAN | 8);    // 0xC8 COM 扫描方向
    OLED_WR_CMD(OLED_CMD_OFFSET);          // 0xD3 显示偏移
    OLED_WR_CMD(0x00);                     //      偏移 0
    OLED_WR_CMD(OLED_CMD_CLOCK);           // 0xD5 时钟
    OLED_WR_CMD(0x80);                     //      分频=1
    OLED_WR_CMD(OLED_CMD_PRECHARGE);       // 0xD9 预充电
    OLED_WR_CMD(0xF1);                     //      Phase1=15, Phase2=1
    OLED_WR_CMD(OLED_CMD_COM_HW);          // 0xDA COM 引脚配置
    OLED_WR_CMD(0x12);                     //      交替 COM
    OLED_WR_CMD(OLED_CMD_VCOMH);           // 0xDB VCOMH
    OLED_WR_CMD(0x30);                     //      ~0.83 x Vcc
    OLED_WR_CMD(OLED_CMD_CHARGE_PUMP);     // 0x8D 电荷泵
    OLED_WR_CMD(0x14);                     //      开启（必须）
    OLED_WR_CMD(OLED_CMD_ON);              // 0xAF 开启显示

    OLED_Clear();                          // 直接清屏（无帧缓冲，立即写 0）
}

// 清屏
void OLED_Clear(void) {
    for (uint8_t p = 0; p < OLED_PAGES; p++) {
        OLED_SetPos(p, 0);
        for (uint8_t c = 0; c < OLED_W; c++)
            OLED_WR_DATA(0x00);
    }
}

// 清指定行
void OLED_ClearLine(uint8_t line) {
    if (line >= OLED_LINES) return;
    for (uint8_t p = line * 2; p < line * 2 + 2 && p < OLED_PAGES; p++) {
        OLED_SetPos(p, 0);
        for (uint8_t c = 0; c < OLED_W; c++)
            OLED_WR_DATA(0x00);
    }
}

//调对比度
void OLED_SetContrast(uint8_t val) {
    OLED_WR_CMD(OLED_CMD_CONTRAST);
    OLED_WR_CMD(val);
}

// 反色
void OLED_InvertDisplay(uint8_t inv) {
    OLED_WR_CMD(inv ? (OLED_CMD_DISP_NORM | 1) : OLED_CMD_DISP_NORM);
}

// 显示单个字符串
void OLED_ShowChar(uint8_t x, uint8_t page, char chr) {
    if (page >= OLED_PAGES - 1) return;    // 需 2 页，page 最大 6
    if (x > OLED_W - 8) return;
    if (chr < ' ' || chr > '~') chr = ' ';  // 超出可显示范围显示空格
    uint16_t base = (uint16_t)(chr - ' ') * 16;

    // 上半页
    OLED_SetPos(page, x);
    for (uint8_t i = 0; i < 8; i++)
        OLED_WR_DATA(F8x16[base + i]);

    // 下半页
    OLED_SetPos(page + 1, x);
    for (uint8_t i = 0; i < 8; i++)
        OLED_WR_DATA(F8x16[base + i + 8]);
}

// 显示字符串
void OLED_ShowString(uint8_t x, uint8_t page, const char *str) {
    while (*str) {
        if (x > OLED_W - 8) {               // 一行写满 -> 换行
            x = 0;
            page += 2;                      // 两页显示出来是一行
            if (page >= OLED_PAGES - 1) break;
        }
        OLED_ShowChar(x, page, *str++);
        x += 8;
    }
}
