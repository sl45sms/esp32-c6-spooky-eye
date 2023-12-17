/*
This file is based on the example code from the ESP-IDF repository, which is licensed under the CC0-1.0 license.
The image used is stored in flash as a jpeg file. This file contains the decode_image routine, 
which uses the tiny JPEG decoder library to decode this JPEG into a format that can be sent to the display.

Keep in mind that the decoder library cannot handle progressive files (will give
``Image decoder: jd_prepare failed (8)`` as an error) so make sure to save in the correct
format if you want to use a different image file.
*/

#include "decode_jpg.h"
#include "jpeg_decoder.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "main.h"

// Reference the binary-included jpeg file
extern const uint8_t image_jpg_start[] asm("_binary_image_logo_240x240_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_image_logo_240x240_jpg_end");
// Define the height and width of the jpeg file. Make sure this matches the actual jpeg
// dimensions.

uint16_t *pixels;

const char *TAG = "JPEGDec";

// Decode the embedded image into pixel lines that can be used with the rest of the logic.
esp_err_t decode_jpg(uint16_t **buffer)
{
    *buffer = NULL;
    esp_err_t ret = ESP_OK;
    // Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    *buffer = calloc(IMAGE_H * IMAGE_W, sizeof(uint16_t));
    ESP_GOTO_ON_FALSE((*buffer), ESP_ERR_NO_MEM, err, TAG, "Error allocating memory for lines");

    // JPEG decode config
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)image_jpg_start,
        .indata_size = image_jpg_end - image_jpg_start,
        .outbuf = (uint8_t *)(*buffer),
        .outbuf_size = IMAGE_W * IMAGE_H * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 1,
        }};

    // JPEG decode
    esp_jpeg_image_output_t outimg;
    esp_jpeg_decode(&jpeg_cfg, &outimg);

    ESP_LOGI(TAG, "JPEG image decoded! Size of the decoded image is: %dpx x %dpx", outimg.width, outimg.height);

    return ESP_OK;
err:
    // Something went wrong! Exit cleanly, de-allocating everything we allocated.
    if (*buffer != NULL)
    {
        free(*buffer);
    }
    return ret;
}

// Grab a rgb16 pixel from the image
static inline uint16_t get_bgnd_pixel(int x, int y, uint16_t *pixelsbuffer)
{
    // Get color of the pixel on x,y coords
    uint16_t color = (uint16_t) * (pixelsbuffer + (y * IMAGE_W) + x);
    // return the color inverted (black to white, white to black)
    return ~color;
}

// Simple routine to decode and send jpg to the LCD.
esp_err_t display_jpg(esp_lcd_panel_handle_t panel_handle)
{
    ESP_ERROR_CHECK(decode_jpg(&pixels));
    // buffer to store the converted pixels
    static uint16_t *color;
    // Allocate memory for the color buffer
    color = heap_caps_malloc(ST7789_LCD_V_RES * ST7789_LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    assert(color != NULL);
    if (color == NULL)
        {
            ESP_LOGE(TAG, "Failed to allocate memory for color array");
            return ESP_FAIL;
        }
    // Calculate a line
    for (int cy = 0; cy < ST7789_LCD_V_RES; cy++)
    { // Calculate a pixel
        for (int cx = 0; cx < ST7789_LCD_H_RES; cx++)
        {
            color[cx + (cy)*ST7789_LCD_H_RES] = get_bgnd_pixel(cx, cy, pixels);
        }
    }

    // Send the calculated data
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, ST7789_LCD_H_RES, ST7789_LCD_V_RES, color);

    // Free the color buffer
    free(color);
    // Free the pixel buffer
    free(pixels);
    return ESP_OK;
}
