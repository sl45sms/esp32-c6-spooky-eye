| Supported Target ESP32-C6 | 

Board used: ESP32-C6-DevKitC-1-N8
LCD: ST7789 240x240 1.3" IPS LCD Display (SPI) CS is always low on this LCD

## how looks like
youtube:
https://youtu.be/hauD0UXycrw


## ST7789, tjpgd and libnsgif Example
This is a simplified example of using the tjpgd and also libnsgif to decode JPEG and GIF images, and display them on an LCD screen.

### Hardware Required

* An ESP-C6 development board
* An SPI-interfaced LCD screen (ST7789)
* An USB cable for power supply and programming

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
      ESP Board                            LCD Screen
      +---------+              +---------------------------------+
      |         |              |                                 |
      |     GND +--------------+ GND   +----------------------+  |
      |         |              |       |                      |  |
      |     3V3 +--------------+ VCC   |                      |  |
      |         |              |       |                      |  |
      |      19 +--------------+ SCL   |                      |  |
      |         |              |       |                      |  |
      |      18 +--------------+ SDA   |                      |  |
      |         |              |       |                      |  |
      |      10 +--------------+ RES   |                      |  |
      |         |              |       |                      |  |
      |      11 +--------------+ DC    |                      |  |
      |         |              |       |                      |  |
      |       2 +--------------+ BCKL  +----------------------+  |
      |         |              |                                 |
      +---------+              +---------------------------------+
```

The GPIO numbers used by this example can be changed in [main.h](main/main.h), where:

| GPIO number              | LCD pin |
| ------------------------ | ------- |
| ST7789_PIN_NUM_PCLK      | SCL     |
| ST7789_PIN_NUM_DC        | DC      |
| ST7789_PIN_NUM_RST       | RES     |
| ST7789_PIN_NUM_DATA0     | SDA     |
| ST7789_PIN_NUM_BK_LIGHT  | BCKL    |


### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project. The JPG logo picture will be shown on the LCD screen. After 3 seconds, the picture will be changed to the annimated GIF picture.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

