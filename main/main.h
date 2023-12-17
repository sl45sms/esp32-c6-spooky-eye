//if not defined, define
#ifndef MAIN_H
#define MAIN_H

// Using SPI2 in the example
#define LCD_HOST    SPI2_HOST

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ST7789_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define ST7789_LCD_BK_LIGHT_ON_LEVEL  1 //MOD
#define ST7789_LCD_BK_LIGHT_OFF_LEVEL !ST7789_LCD_BK_LIGHT_ON_LEVEL
#define ST7789_PIN_NUM_DATA0          18//MOD  /*!< for 1-line SPI, this also refereed as MOSI */
#define ST7789_PIN_NUM_PCLK           19//MOD
#define ST7789_PIN_NUM_CS             -1//MOD
#define ST7789_PIN_NUM_DC             11//MOD
#define ST7789_PIN_NUM_RST            10//MOD
#define ST7789_PIN_NUM_BK_LIGHT       2//MOD

// The pixel number in horizontal and vertical
#define ST7789_LCD_H_RES              240//MOD 240 for ST7789
#define ST7789_LCD_V_RES              240
// Bit number used to represent command and parameter
#define ST7789_LCD_CMD_BITS           8
#define ST7789_LCD_PARAM_BITS         8

// end

#endif // MAIN_H



