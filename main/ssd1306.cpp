#include "ssd1306.h"

namespace SSD1306
{
    bool oled::_running{false};
    i2c_port_t oled::_bus;
    uint16_t oled::_devAddr;
    uint8_t oled::_displayBuffer[128][8];

    oled::oled(){};

    esp_err_t oled::_write_data(const uint8_t *data, const uint16_t len)
    {
        esp_err_t status;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        status = i2c_master_start(cmd);
        if (ESP_OK == status)
            status = i2c_master_write_byte(cmd, _devAddr | I2C_MASTER_WRITE, true);
        if (ESP_OK == status)
            status = i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
        if (ESP_OK == status)
            status = i2c_master_write(cmd, data, len, true);
        if (ESP_OK == status)
            status = i2c_master_stop(cmd);
        if (ESP_OK == status)
            status = i2c_master_cmd_begin(_bus, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return status;
    }

    void oled::clear_screen(uint8_t cFill)
    {
        memset(_displayBuffer, cFill, sizeof(_displayBuffer));
    }

    esp_err_t oled::refresh()
    {
        return _write_data(&_displayBuffer[0][0], sizeof(_displayBuffer));
    }

    esp_err_t oled::_init() // swap out commands in comments for 180(deg) rotation
    {
        esp_err_t status;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, _devAddr | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
        i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_START_LINE, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
        i2c_master_write_byte(cmd, 0xFF, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_0, true); // OLED_CMD_SET_SEGMENT_REMAP_0
        i2c_master_write_byte(cmd, 0xC8, true);                         // 0xC0
        i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_MUX_RATIO, true);
        i2c_master_write_byte(cmd, 0x3F, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);
        i2c_master_write_byte(cmd, 0x80, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_PRECHARGE, true);
        i2c_master_write_byte(cmd, 0xF1, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_COM_PIN_MAP, true);
        i2c_master_write_byte(cmd, 0x02, true); // 0x22
        i2c_master_write_byte(cmd, OLED_CMD_SET_VCOMH_DESELCT, true);
        i2c_master_write_byte(cmd, 0x40, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
        i2c_master_write_byte(cmd, 0x14, true);
        i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_RAM, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_MEMORY_ADDR_MODE, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_VERT_ADDR_MODE, true);
        i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
        i2c_master_stop(cmd);

        status = i2c_master_cmd_begin(_bus, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return status;
    }

    esp_err_t oled::init()
    {
        i2c_config_t cfg = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = static_cast<gpio_num_t>(CONFIG_ESP_OLED_SDA_GPIO_NUM),
            .scl_io_num = static_cast<gpio_num_t>(CONFIG_ESP_OLED_SCL_GPIO_NUM),
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {
                .clk_speed = CONFIG_ESP_OLED_I2C_MASTER_FREQ_HZ,
            },
            .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
        };
        _bus = I2C_NUM_0;
        _devAddr = OLED_I2C_ADDRESS << 1;

        esp_err_t status = i2c_param_config(_bus, &cfg);
        if (ESP_OK == status)
            status = i2c_driver_install(_bus, cfg.mode, 0, 0, 0);
        if (ESP_OK == status)
            status = _init();

        if (ESP_OK == status)
        {
            clear_screen();
            refresh();
            _running = true;
            ESP_LOGI("OLED", "Initialised");
        }

        return status;
    }

    esp_err_t oled::power_down()
    {
        esp_err_t status;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, _devAddr | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
        i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
        i2c_master_stop(cmd);

        status = i2c_master_cmd_begin(_bus, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ESP_OK == status)
            _running = false;
        return status;
    }

    void oled::fill_point(uint8_t xPos, uint8_t yPos, bool point)
    {
        if (xPos >= CONFIG_ESP_OLED_WIDTH || yPos >= CONFIG_ESP_OLED_HEIGHT)
            return;

        uint8_t pos = 7 - yPos / 8;
        uint8_t bx = yPos % 8;
        uint8_t temp = 1 << (7 - bx);

        if (point)
            _displayBuffer[xPos][pos] |= temp;
        else
            _displayBuffer[xPos][pos] &= ~temp;
    }

    void oled::draw_7seg(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point)
    {
        uint8_t cTemp, yPos0 = yPos;
        for (uint8_t i = 0; i < (cWidth == 32 ? sizeof(sevenSeg3216[0]) : sizeof(sevenSeg1608[0])); i++)
        {
            if (cWidth == 32)
                cTemp = sevenSeg3216[cChar - '0'][i];
            else
                cTemp = sevenSeg1608[cChar - '0'][i];
            if (!point)
                cTemp = ~cTemp;
            for (uint8_t j = 0; j < 8; j++)
            {
                fill_point(xPos, yPos, cTemp & 0x80);
                cTemp <<= 1;
                yPos++;
                if ((yPos - yPos0) == cWidth)
                {
                    yPos = yPos0;
                    xPos++;
                }
            }
        }
    }

    void oled::draw_char(uint8_t xPos, uint8_t yPos, uint8_t cChar, uint8_t cWidth, bool point)
    {
        uint8_t cTemp, yPos0 = yPos;
        for (uint8_t i = 0; i < (cWidth == 32 ? sizeof(c_chFont1608[0]) : sizeof(c_chFont1206[0])); i++)
        {
            if (cWidth == 16)
                cTemp = c_chFont1608[cChar - ' '][i];
            else
                cTemp = c_chFont1206[cChar - ' '][i];
            if (!point)
                cTemp = ~cTemp;
            for (uint8_t j = 0; j < 8; j++)
            {
                fill_point(xPos, yPos, cTemp & 0x80);
                cTemp <<= 1;
                yPos++;
                if ((yPos - yPos0) == cWidth)
                {
                    yPos = yPos0;
                    xPos++;
                }
            }
        }
    }

    void oled::drawTime(const char *SNTP_time)
    {
        draw_7seg(0, 0, SNTP_time[11], 32, 1); // hour
        draw_7seg(20, 0, SNTP_time[12], 32, 1);
        draw_7seg(40, 0, SNTP_time[14], 32, 1); // minute
        draw_7seg(60, 0, SNTP_time[15], 32, 1);
        draw_7seg(80, 0, SNTP_time[17], 16, 1); // second
        draw_7seg(90, 0, SNTP_time[18], 16, 1);
        draw_char(104, 0, SNTP_time[0], 16, 1); // day
        draw_char(112, 0, SNTP_time[1], 16, 1);
        draw_char(120, 0, SNTP_time[2], 16, 1);
        draw_char(84, 16, SNTP_time[8], 16, 1); // date
        draw_char(94, 16, SNTP_time[9], 16, 1);
        draw_char(104, 16, SNTP_time[4], 16, 1); // month
        draw_char(112, 16, SNTP_time[5], 16, 1);
        draw_char(120, 16, SNTP_time[6], 16, 1);
    }

    void oled::fill_rectangle(uint8_t xPos1, uint8_t xPos2, uint8_t yPos1, uint8_t yPos2, bool point)
    {
        if (xPos1 > xPos2)
        {
            uint8_t temp = xPos1;
            xPos1 = xPos2;
            xPos2 = temp;
        }
        if (yPos1 > yPos2)
        {
            uint8_t temp = yPos1;
            yPos1 = yPos2;
            yPos2 = temp;
        }

        for (uint8_t x = xPos1; x <= xPos2; x++)
        {
            for (uint8_t y = yPos1; y <= yPos2; y++)
            {
                fill_point(x, y, point);
            }
        }
    }
}
