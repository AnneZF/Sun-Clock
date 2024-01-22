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
        static esp_err_t _init();

    public:
        oled();
        /**
         * @brief Initialises OLED
         *
         * @return
         *      - ESP_OK if initialised successfully
         */
        static esp_err_t init();

        /**
         * @brief Sends buffer to OLED and displays
         *
         * @return
         *      - ESP_OK if initialised successfully
         */
        static esp_err_t refresh();

        /**
         * @brief Turn off OLED
         *
         * @return
         *      - ESP_OK if initialised successfully
         */
        static esp_err_t power_down();

        /**
         * @brief Fill buffer with character
         *
         * @param[in] cFill character to fill with
         */
        static void clear_screen(uint8_t cFill = 0x00);

        /**
         * @brief gets OLED state
         *
         * @return
         *      - true if running
         */
        static bool oled_state() { return _running; }

        /**
         * @brief Fills (x, y) point
         *
         * @param[in] xPos x position
         * @param[in] yPos y position
         * @param[in] point true - on, false - off
         */
        static void fill_point(uint8_t xPos, uint8_t yPos, bool point);

        /**
         * @brief Fills rectangle
         *
         * @param[in] xPos1 x1 position
         * @param[in] xPos2 x2 position
         * @param[in] yPos1 y1 position
         * @param[in] yPos2 y2 position
         * @param[in] point true - on, false - off
         */
        static void fill_rectangle(uint8_t xPos1, uint8_t xPos2, uint8_t yPos1, uint8_t yPos2, bool point);

        /**
         * @brief Draw 7-segment number
         *
         * @param[in] xPos x position (top-left)
         * @param[in] yPos y position (top-left)
         * @param[in] cChar ascii character
         * @param[in] cWidth character width - 32 or 16
         * @param[in] point true - on, false - off
         */
        static void draw_7seg(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point);

        /**
         * @brief Draw character
         *
         * @param[in] xPos x position (top-left)
         * @param[in] yPos y position (top-left)
         * @param[in] cChar ascii character
         * @param[in] cWidth character width - 16 or 8
         * @param[in] point true - on, false - off
         */
        static void draw_char(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point);

        /**
         * @brief Draw time
         *
         * @param[in] SNTP_time asctime
         */
        static void drawTime(const char *SNTP_time);
    };
}