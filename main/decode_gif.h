#pragma once
#include <stdint.h>
#include "esp_err.h"

#define IMAGE_W 240
#define IMAGE_H 240

esp_err_t gif_prepare();
esp_err_t display_gif_run();

/**
 * @brief Decode the jpeg ``blackeye.gif`` embedded into the program file into pixel data.
 *
 * @param pixels A pointer to a pointer for an array of rows, which themselves are an array of pixels.
 *        Effectively, you can get the pixel data by doing ``decode_image(&myPixels); pixelval=myPixels[ypos][xpos];``
 * @return - ESP_ERR_NOT_SUPPORTED if gif is malformed
 *         - ESP_ERR_NO_MEM if out of memory
 *         - ESP_OK on succesful decode
 */
esp_err_t decode_gif(uint16_t **pixels);