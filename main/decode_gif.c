#include "decode_gif.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <nsgif.h>
#include "main.h"

#define BYTE_SWAP(v) ((v >> 8) | (v << 8))

extern const uint8_t embed_blackeye_start[] asm("_binary_blackeye_gif_start");
extern const uint8_t embed_blackeye_end[] asm("_binary_blackeye_gif_end");

nsgif_t *gif = NULL;
const nsgif_info_t *gif_info;
uint32_t next_frame_time = 0;

const char *GTAG = "GIFDec";

static nsgif_bitmap_t *bitmap_create(int width, int height)
{
    return calloc(width * height, 4);
}

static void bitmap_destroy(nsgif_bitmap_t *bitmap)
{
    free(bitmap);
}

static uint8_t *bitmap_get_buffer(nsgif_bitmap_t *bitmap)
{
    return (uint8_t *)bitmap;
}

static void bitmap_set_opaque(void *bitmap, bool opaque) {}

static bool bitmap_test_opaque(void *bitmap)
{
    return false;
}

static void bitmap_mark_modified(void *bitmap) {}

static nsgif_bitmap_cb_vt bitmap_callbacks = {
    .create = bitmap_create,
    .destroy = bitmap_destroy,
    .get_buffer = bitmap_get_buffer,
    .set_opaque = bitmap_set_opaque,
    .test_opaque = bitmap_test_opaque,
    .modified = bitmap_mark_modified,
    .get_rowspan = NULL};

esp_err_t gif_prepare()
{
    // clean up
    if (gif)
        nsgif_destroy(gif);

    nsgif_error res = nsgif_create(&bitmap_callbacks, NSGIF_BITMAP_FMT_RGBA8888, &gif);
    if (res != NSGIF_OK)
    {
        ESP_LOGE(GTAG, "Error creating GIF handler: %d (%s)", res, nsgif_strerror(res));
        return ESP_FAIL;
    }

    size_t size = embed_blackeye_end - embed_blackeye_start;
    res = nsgif_data_scan(gif, size, (uint8_t *)embed_blackeye_start);
    nsgif_data_complete(gif);

    if (res != NSGIF_OK)
    {
        nsgif_destroy(gif);
        gif = NULL;
        ESP_LOGE(GTAG, "Error loading GIF: %d (%s)", res, nsgif_strerror(res));
        return ESP_FAIL;
    }

    gif_info = nsgif_get_info(gif);
    ESP_LOGD(GTAG, "GIF frame_count=%lu, width=%lu, height=%lu", gif_info->frame_count, gif_info->width, gif_info->height);

    next_frame_time = 0;

    return ESP_OK;
}

// Convert 32bit RGBA8888 to 16bit RGB565
static inline u_int16_t ConvertRGB(u_int32_t color)
{
    u_int16_t r = (color >> 24) & 0xFF;
    u_int16_t g = (color >> 16) & 0xFF;
    u_int16_t b = (color >> 8) & 0xFF;
    u_int16_t rgb = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    return ~BYTE_SWAP(rgb);
}

esp_err_t display_gif_run(esp_lcd_panel_handle_t panel_handle)
{
    ESP_ERROR_CHECK(gif_prepare());

    if (!gif)
        return ESP_OK;

    // render frames
    for (int frameno = 0; frameno < gif_info->frame_count; frameno++)
    {

        uint32_t time_cs = esp_timer_get_time() / 10000;
        if (time_cs < next_frame_time)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            time_cs = esp_timer_get_time() / 10000;
        }

        nsgif_rect_t frame_rect;
        uint32_t delay_cs;
        uint32_t frame;
        nsgif_error res = nsgif_frame_prepare(gif, &frame_rect, &delay_cs, &frame);
        if (res == NSGIF_ERR_ANIMATION_END)
        {
            nsgif_reset(gif);
            res = nsgif_frame_prepare(gif, &frame_rect, &delay_cs, &frame);
        }
        if (res != NSGIF_OK)
        {
            ESP_LOGW(GTAG, "Error preparing GIF frame: %d (%s)", res, nsgif_strerror(res));
            return ESP_FAIL;
        }
        next_frame_time = delay_cs == NSGIF_INFINITE ? UINT32_MAX : time_cs + delay_cs - 1;
        next_frame_time += 1250;
        nsgif_bitmap_t *bitmap;
        res = nsgif_frame_decode(gif, frame, &bitmap);
        if (res != NSGIF_OK)
        {
            ESP_LOGW(GTAG, "Error decoding GIF frame: %d (%s)", res, nsgif_strerror(res));
            return ESP_FAIL;
        }

        uint32_t *frame_image = (uint32_t *)bitmap; // 32bpp, 8 bits per channel bitmap pixel format
        // alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    static uint16_t *color;
    // Allocate memory for the color buffer
    color = heap_caps_malloc(ST7789_LCD_V_RES * ST7789_LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    assert(color != NULL); // Check if memory allocation was successful
        if (color == NULL)
        {
            ESP_LOGE(GTAG, "Failed to allocate memory for color array");
            return ESP_FAIL;
        }
        // Iterate over the frame image
        for (size_t x = 0; x < ST7789_LCD_H_RES; x++)
        {
            for (size_t y = 0; y < ST7789_LCD_V_RES; y++)
            {
                // Assign the converted color to the correct element in the color array
                color[y * ST7789_LCD_H_RES + x] = ConvertRGB(frame_image[y * ST7789_LCD_H_RES + x]);
            }
        }
        esp_lcd_panel_draw_bitmap(panel_handle,
                                  0,
                                  0,
                                  ST7789_LCD_H_RES,
                                  ST7789_LCD_V_RES,
                                  color);
        vTaskDelay(250 / portTICK_PERIOD_MS); // TODO this removes the flickering...
        // and slows the rendering, but it's not the best solution
        free(color);
    }
    return ESP_OK;
}
