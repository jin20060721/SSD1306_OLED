#include "OLED_Display.h"
#include "font.h"
#include "mbed.h"

// 外部I2C对象，注意一定要和主函数中的I2C定义名保持一致
extern I2C i2c;

// 设置的显存
uint8_t OLED_GRAM[OLED_PAGES][OLED_W];

// 底层写命令
void OLED_WR_CMD(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c.write(OLED_I2C_ADDR, (char *)buf, 2);
}


// 初始化，务必要加入在主函数开头
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

    OLED_Clear();                          // 清空显存
    OLED_Refresh();                        // 推送到屏幕
}

// 刷新，把显存推送给显示屏
void OLED_Refresh(void) {
    uint8_t buf[OLED_W + 1];
    buf[0] = 0x40;                         // 数据控制字节
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_WR_CMD(0xB0 + page);          // 选页
        OLED_WR_CMD(0x00);                 // 列低 4 位
        OLED_WR_CMD(0x10);                 // 列高 4 位
        for (uint8_t c = 0; c < OLED_W; c++)
            buf[1 + c] = OLED_GRAM[page][c];
        i2c.write(OLED_I2C_ADDR, (char *)buf, OLED_W + 1);
    }
}

// 清屏
void OLED_Clear(void) {
    for (uint8_t p = 0; p < OLED_PAGES; p++)
        for (uint8_t c = 0; c < OLED_W; c++)
            OLED_GRAM[p][c] = 0;
#if OLED_AUTO_REFRESH
    OLED_Refresh();
#endif
}

// 清除某行文本（0~3）
void OLED_ClearLine(uint8_t line) {
    if (line >= OLED_PAGES / 2) return;
    for (uint8_t p = line * 2; p < line * 2 + 2 && p < OLED_PAGES; p++)
        for (uint8_t c = 0; c < OLED_W; c++)
            OLED_GRAM[p][c] = 0;
#if OLED_AUTO_REFRESH
    OLED_Refresh();
#endif
}

// 调整对比度（0~255），默认最亮255
void OLED_SetContrast(uint8_t val) {
    OLED_WR_CMD(OLED_CMD_CONTRAST);
    OLED_WR_CMD(val);
}

// 反色显示
void OLED_InvertDisplay(uint8_t inv) {
    OLED_WR_CMD(inv ? (OLED_CMD_DISP_NORM | 1) : OLED_CMD_DISP_NORM);
}

// 在指定位置显示单个字符
void OLED_ShowChar(uint8_t x, uint8_t y, char chr) {
    if (y >= OLED_PAGES - 1) return;
    if (x > OLED_W - 8) return;
    if (chr < ' ' || chr > '~') chr = ' ';  // 超出范围显示空格
    uint16_t base = (uint16_t)(chr - ' ') * 16;
    for (uint8_t i = 0; i < 8; i++) {
        OLED_GRAM[y][x + i]     = F8x16[base + i];       // 上半页（行 0~7）
        OLED_GRAM[y + 1][x + i] = F8x16[base + i + 8];   // 下半页（行 8~15）
    }
#if OLED_AUTO_REFRESH
    OLED_Refresh();
#endif
}

// 在指定位置生成字符串
void OLED_ShowString(uint8_t x, uint8_t y, const char *str) {
    while (*str) {
        if (x > OLED_W - 8) {               // 一行写满 -> 换到下一文本行
            x = 0;
            y += 2;
            if (y >= OLED_PAGES - 1) break;
        }
        OLED_ShowChar(x, y, *str++);
        x += 8;
    }
#if OLED_AUTO_REFRESH
    OLED_Refresh();
#endif
}
