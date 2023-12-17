/*
This file is based on the example code from the ESP-IDF repository, which is licensed under the CC0-1.0 license.
*/

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "decode_jpg.h"
#include "decode_gif.h"
#include "main.h"

void app_main(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << ST7789_PIN_NUM_BK_LIGHT
    };
    // Initialize the GPIO of backlight
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    spi_bus_config_t buscfg = {
        .sclk_io_num = ST7789_PIN_NUM_PCLK,
        .mosi_io_num = ST7789_PIN_NUM_DATA0,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7789_LCD_V_RES * ST7789_LCD_H_RES * 2 + 8
    };
    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = ST7789_PIN_NUM_DC,
        .cs_gpio_num = ST7789_PIN_NUM_CS,
        .pclk_hz = ST7789_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = ST7789_LCD_CMD_BITS,
        .lcd_param_bits = ST7789_LCD_PARAM_BITS,
        .spi_mode = 3, //sould be 3 for ST7789
        .trans_queue_depth = 10,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = ST7789_PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    // Initialize the LCD configuration
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    // Turn off backlight to avoid unpredictable display on the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(ST7789_PIN_NUM_BK_LIGHT, ST7789_LCD_BK_LIGHT_OFF_LEVEL));
    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // Turn on the screen
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

// unused but can be used to invert the color of the screen
//ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
//ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
//ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));

// Turn on backlight (Different LCD screens may need different levels)
ESP_ERROR_CHECK(gpio_set_level(ST7789_PIN_NUM_BK_LIGHT, ST7789_LCD_BK_LIGHT_ON_LEVEL));

// decode the image logo
ESP_ERROR_CHECK(display_jpg(panel_handle));

// Wait for 3 seconds
vTaskDelay(3000 / portTICK_PERIOD_MS);

// Start the infinite loop gif display
    while (1) {
        ESP_ERROR_CHECK(display_gif_run(panel_handle));
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}