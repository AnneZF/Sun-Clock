#pragma once

#include "driver/i2c.h"
#include "stdint.h"
#include "ssd1306_fonts.h"
#include "string.h"
#include "esp_log.h"

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS 0x3C

/* Control byte for i2c
Co : bit 8 : Continuation Bit
 * 1 = no-continuation (only one byte to follow)
 * 0 = the controller should expect a stream of bytes.
D/C# : bit 7 : Data/Command Select bit
 * 1 = the next byte or byte stream will be Data.
 * 0 = a Command byte or byte stream will be coming up next.
 Bits 6-0 will be all zeros.
Usage:
0x80 : Single Command byte
0x00 : Command Stream
0xC0 : Single Data byte
0x40 : Data Stream
*/
#define OLED_CONTROL_BYTE_CMD_SINGLE 0x80
#define OLED_CONTROL_BYTE_CMD_STREAM 0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE 0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST 0x81 // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM 0xA4
#define OLED_CMD_DISPLAY_ALLON 0xA5
#define OLED_CMD_DISPLAY_NORMAL 0xA6
#define OLED_CMD_DISPLAY_INVERTED 0xA7
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_DISPLAY_ON 0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE 0x20
#define OLED_CMD_SET_HORI_ADDR_MODE 0x00 // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE 0x01 // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE 0x02 // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE 0x21   // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE 0x22     // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0 0xA0
#define OLED_CMD_SET_SEGMENT_REMAP_1 0xA1
#define OLED_CMD_SET_MUX_RATIO 0xA8 // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE 0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3 // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP 0xDA    // follow with 0x12
#define OLED_CMD_NOP 0xE3                // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV 0xD5 // follow with 0x80
#define OLED_CMD_SET_PRECHARGE 0xD9       // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT 0xDB   // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP 0x8D // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT 0x26
#define OLED_CMD_HORIZONTAL_LEFT 0x27
#define OLED_CMD_CONTINUOUS_SCROLL 0x29
#define OLED_CMD_DEACTIVE_SCROLL 0x2E
#define OLED_CMD_ACTIVE_SCROLL 0x2F
#define OLED_CMD_VERTICAL 0xA3

#define COORDINATE_SWAP(x1, x2, y1, y2) \
    {                                   \
        int16_t temp = x1;              \
        x1 = x2, x2 = temp;             \
        temp = y1;                      \
        y1 = y2;                        \
        y2 = temp;                      \
    }

namespace SSD1306
{
    class oled
    {
    private:
        static bool _running;
        static i2c_port_t _bus;
        static uint16_t _devAddr;
        static uint8_t _displayBuffer[128][8];
        static esp_err_t _write_data(const uint8_t *data, const uint16_t len);
        // static esp_err_t _write_cmd(const uint8_t *data, const uint16_t len);
        // static esp_err_t _write_cmd_byte(const uint8_t data);
        static esp_err_t _init();

    public:
        oled();
        static esp_err_t init();
        static esp_err_t refresh();
        static esp_err_t power_down();
        static void clear_screen(uint8_t chFill = 0x00);
        static bool oled_state() { return _running; }
        static void fill_point(uint8_t xPos, uint8_t yPos, bool point);
        static void fill_rectangle(uint8_t xPos1, uint8_t xPos2, uint8_t yPos1, uint8_t yPos2, bool point); 
        static void draw_7seg(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point);
        static void draw_char(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point);
        static void drawTime(const char *SNTP_time);
    };
}

// /**
//  * @brief   device initialization
//  *
//  * @param   dev object handle of ssd1306
//  *
//  * @return
//  *     - ESP_OK Success
//  *     - ESP_FAIL Fail
//  */
// esp_err_t ssd1306_init(ssd1306_handle_t dev);

// /**
//  * @brief   Create and initialization device object and return a device handle
//  *
//  * @param   port     I2C port object handle
//  * @param   dev_addr I2C device address of device
//  *
//  * @return
//  *     - device object handle of ssd1306
//  */
// ssd1306_handle_t ssd1306_create(i2c_port_t port, uint16_t dev_addr);

// /**
//  * @brief   Delete and release a device object
//  *
//  * @param   dev object handle of ssd1306
//  */
// void ssd1306_delete(ssd1306_handle_t dev);

// /**
//  * @brief   draw point on (x, y)
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   chPoint fill point
//  */
// void ssd1306_fill_point(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, uint8_t chPoint);

// /**
//  * @brief   Draw rectangle on (x1,y1)-(x2,y2)
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos1
//  * @param   chYpos1
//  * @param   chXpos2
//  * @param   chYpos2
//  * @param   chDot fill point
//  */
// void ssd1306_fill_rectangle(ssd1306_handle_t dev, uint8_t chXpos1, uint8_t chYpos1,
//                             uint8_t chXpos2, uint8_t chYpos2, uint8_t chDot);

// /**
//  * @brief   display char on (x, y),and set size, mode
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   chSize char size
//  * @param   chChr draw char
//  * @param   chMode display mode
//  */
// void ssd1306_draw_char(ssd1306_handle_t dev, uint8_t chXpos,
//                        uint8_t chYpos, uint8_t chChr, uint8_t chSize, uint8_t chMode);

// /**
//  * @brief   display number on (x, y),and set length, size, mode
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   chNum draw num
//  * @param   chLen length
//  * @param   chSize display size
//  */
// void ssd1306_draw_num(ssd1306_handle_t dev, uint8_t chXpos,
//                       uint8_t chYpos, uint32_t chNum, uint8_t chLen, uint8_t chSize);

// /**
//  * @brief   display 1616char on (x, y)
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   chChar draw char
//  */
// void ssd1306_draw_1616char(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, uint8_t chChar);

// /**
//  * @brief   display 3216char on (x, y)
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   chChar draw char
//  */
// void ssd1306_draw_3216char(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, uint8_t chChar);

// /**
//  * @brief   draw bitmap on (x, y),and set width, height
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   pchBmp point to BMP data
//  * @param   chWidth picture width
//  * @param   chHeight picture heght
//  */
// void ssd1306_draw_bitmap(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
//                          const uint8_t *pchBmp, uint8_t chWidth, uint8_t chHeight);

// /**
//  * @brief   draw line between two specified points
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos1 Specifies the X position of the starting point of the line
//  * @param   chYpos1 Specifies the Y position of the starting point of the line
//  * @param   chXpos2 Specifies the X position of the ending point of the line
//  * @param   chYpos2 Specifies the Y position of the ending point of the line
//  */
// void ssd1306_draw_line(ssd1306_handle_t dev, int16_t chXpos1, int16_t chYpos1, int16_t chXpos2, int16_t chYpos2);

// /**
//  * @brief   refresh dot matrix panel
//  *
//  * @param   dev object handle of ssd1306

//  * @return
//  *     - ESP_OK Success
//  *     - ESP_FAIL Fail
//  **/
// esp_err_t ssd1306_refresh_gram(ssd1306_handle_t dev);

// /**
//  * @brief   Clear screen
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chFill whether fill and fill char
//  **/
// void ssd1306_clear_screen(ssd1306_handle_t dev, uint8_t chFill);

// /**
//  * @brief   Displays a string on the screen
//  *
//  * @param   dev object handle of ssd1306
//  * @param   chXpos Specifies the X position
//  * @param   chYpos Specifies the Y position
//  * @param   pchString Pointer to a string to display on the screen
//  * @param   chSize char size
//  * @param   chMode display mode
//  **/
// void ssd1306_draw_string(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
//                          const uint8_t *pchString, uint8_t chSize, uint8_t chMode);
