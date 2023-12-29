#include "led_strip.h"

namespace LED
{
    bool Led::_running{false};
    rmt_channel_handle_t Led::_led_channel;
    rmt_encoder_handle_t Led::_led_encoder;
    rmt_transmit_config_t Led::_tx_config = {
        .loop_count = 0,
    };
    u_int8_t Led::_led_pixels[CONFIG_ESP_LED_NUMBERS * 3];

    Led::Led(){};

    size_t Led::_rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
    {
        rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
        rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
        rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
        rmt_encode_state_t session_state = RMT_ENCODING_RESET;
        int state = RMT_ENCODING_RESET;
        size_t encoded_symbols = 0;
        switch (led_encoder->state)
        {
        case RMT_ENCODING_RESET: // send RGB data
            encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
            if (session_state & RMT_ENCODING_COMPLETE)
            {
                led_encoder->state = 1; // switch to next state when current encoding session finished
            }
            if (session_state & RMT_ENCODING_MEM_FULL)
            {
                state |= RMT_ENCODING_MEM_FULL;
                goto out; // yield if there's no free space for encoding artifacts
            }             // fall-through
        case RMT_ENCODING_COMPLETE:           // send reset code
            encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                                    sizeof(led_encoder->reset_code), &session_state);
            if (session_state & RMT_ENCODING_COMPLETE)
            {
                led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
                state |= RMT_ENCODING_COMPLETE;
            }
            if (session_state & RMT_ENCODING_MEM_FULL)
            {
                state |= RMT_ENCODING_MEM_FULL;
                goto out; // yield if there's no free space for encoding artifacts
            }
        }
    out:
        *ret_state = (rmt_encode_state_t)state;
        return encoded_symbols;
    }

    esp_err_t Led::_rmt_del_led_strip_encoder(rmt_encoder_t *encoder)
    {
        rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
        rmt_del_encoder(led_encoder->bytes_encoder);
        rmt_del_encoder(led_encoder->copy_encoder);
        free(led_encoder);
        return ESP_OK;
    }

    esp_err_t Led::_rmt_led_strip_encoder_reset(rmt_encoder_t *encoder)
    {
        rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
        rmt_encoder_reset(led_encoder->bytes_encoder);
        rmt_encoder_reset(led_encoder->copy_encoder);
        led_encoder->state = RMT_ENCODING_RESET;
        return ESP_OK;
    }

    esp_err_t Led::_rmt_new_led_strip_encoder(rmt_encoder_handle_t *ret_encoder)
    {
        esp_err_t status{ESP_OK};
        rmt_led_strip_encoder_t *led_encoder = (rmt_led_strip_encoder_t *)calloc(1, sizeof(rmt_led_strip_encoder_t));
        if (!led_encoder)
            status = ESP_ERR_NO_MEM;

        if (ESP_OK == status)
        {
            led_encoder->base.encode = _rmt_encode_led_strip;
            led_encoder->base.del = _rmt_del_led_strip_encoder;
            led_encoder->base.reset = _rmt_led_strip_encoder_reset;
            // different led strip might have its own timing requirements, following parameter is for WS2812
            rmt_bytes_encoder_config_t bytes_encoder_config = {
                .bit0 = {
                    .duration0 = 3 * CONFIG_ESP_LED_STRIP_RESOLUTION / 10000000, // T0H=0.3us
                    .level0 = 1,
                    .duration1 = 9 * CONFIG_ESP_LED_STRIP_RESOLUTION / 10000000, // T0L=0.9us
                    .level1 = 0,
                },
                .bit1 = {
                    .duration0 = 9 * CONFIG_ESP_LED_STRIP_RESOLUTION / 10000000, // T1H=0.9us
                    .level0 = 1,
                    .duration1 = 3 * CONFIG_ESP_LED_STRIP_RESOLUTION / 10000000, // T1L=0.3us
                    .level1 = 0,
                },
                .flags = {
                    .msb_first = 1, // WS2812 transfer bit order: G7...G0R7...R0B7...B0
                },
            };
            status = rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder);
        }
        if (ESP_OK == status)
        {
            rmt_copy_encoder_config_t copy_encoder_config;
            status = rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder);
        }
        if (ESP_OK == status)
        {
            uint16_t reset_ticks = CONFIG_ESP_LED_STRIP_RESOLUTION / 1000000 * 50 / 2; // reset code duration defaults to 50us
            led_encoder->reset_code = (rmt_symbol_word_t){
                .duration0 = reset_ticks,
                .level0 = 0,
                .duration1 = reset_ticks,
                .level1 = 0,
            };
            *ret_encoder = &led_encoder->base;
        }
        if (ESP_OK != status)
        {
            if (led_encoder)
            {
                if (led_encoder->bytes_encoder)
                {
                    rmt_del_encoder(led_encoder->bytes_encoder);
                }
                if (led_encoder->copy_encoder)
                {
                    rmt_del_encoder(led_encoder->copy_encoder);
                }
                free(led_encoder);
            }
        }
        return status;
    }

    esp_err_t Led::init()
    {
        esp_err_t status{ESP_OK};
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = static_cast<gpio_num_t>(CONFIG_ESP_LED_STRIP_GPIO_NUM),
            .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
            .resolution_hz = CONFIG_ESP_LED_STRIP_RESOLUTION,
            .mem_block_symbols = 64, // increase the block size can make the LED less flickering
            .trans_queue_depth = 4,  // set the number of transactions that can be pending in the background
        };
        status = rmt_new_tx_channel(&tx_chan_config, &_led_channel);
        if (ESP_OK == status)
        {
            status = _rmt_new_led_strip_encoder(&_led_encoder);
        }
        if (ESP_OK == status)
        {
            status = rmt_enable(_led_channel);
        }
        if (ESP_OK == status)
        {
            clear();
            send();
            _running = true;
            ESP_LOGI("LED Strip", "Initialised!");
        }
        return status;
    }

    esp_err_t Led::send()
    {
        esp_err_t status{ESP_OK};
        status = rmt_transmit(_led_channel, _led_encoder, _led_pixels, sizeof(_led_pixels), &_tx_config);
        if (ESP_OK == status)
            status = rmt_tx_wait_all_done(_led_channel, portMAX_DELAY);
        return status;
    }

    void Led::clear()
    {
        memset(_led_pixels, 0, sizeof(_led_pixels));
    }

    float _wrap360(float i)
    {
        while (i < 0)
            i += 360;
        while (i >= 360)
            i -= 360;
        return i;
    }

    float _lim1(float i)
    {
        return i < 0 ? 0 : i > 1 ? 1
                                 : i;
    }

    float _mod(float h, int i)
    {
        while (h > i)
            h -= i;
        return h;
    }

    float _max3(float *r, float *g, float *b)
    {
        return ((*r >= *g && *r >= *b) ? *r : (*g >= *r && *g >= *b) ? *g
                                                                     : *b);
    }

    float _min3(float *r, float *g, float *b)
    {
        return ((*r <= *g && *r <= *b) ? *r : (*g <= *r && *g <= *b) ? *g
                                                                     : *b);
    }

    void Led::_setPixelRGB(u_int16_t pixel, float r, float g, float b)
    {
        u_int16_t pixel_num = pixel * 3;
        _led_pixels[pixel_num++] = static_cast<u_int8_t>(g * 255);
        _led_pixels[pixel_num++] = static_cast<u_int8_t>(r * 255);
        _led_pixels[pixel_num] = static_cast<u_int8_t>(b * 255);
    }

    void Led::setPixelRGB(u_int16_t pixel, float r, float g, float b)
    {
        _setPixelRGB(pixel, _lim1(r), _lim1(g), _lim1(b));
    }

    void Led::_setPixelHCXM(u_int16_t pixel, float H, float c, float x, float m)
    {
        if (H < 1)
        {
            _setPixelRGB(pixel, c + m, x + m, m);
        }
        else if (H < 2)
        {
            _setPixelRGB(pixel, x + m, c + m, m);
        }
        else if (H < 3)
        {
            _setPixelRGB(pixel, m, c + m, x + m);
        }
        else if (H < 4)
        {
            _setPixelRGB(pixel, m, x + m, c + m);
        }
        else if (H < 5)
        {
            _setPixelRGB(pixel, x + m, m, c + m);
        }
        else
        {
            _setPixelRGB(pixel, c + m, m, x + m);
        }
    }

    void Led::_setPixelHSV(u_int16_t pixel, float h, float s, float v)
    {
        float c = v * s;
        float H = h / 60;
        float x = c * (1 - abs(_mod(H, 2) - 1));
        float m = v - c;
        _setPixelHCXM(pixel, H, c, x, m);
    }

    void Led::setPixelHSV(u_int16_t pixel, float h, float s, float v)
    {
        _setPixelHSV(pixel, _wrap360(h), _lim1(s), _lim1(v));
    }

    void Led::_setPixelHSL(u_int16_t pixel, float h, float s, float l)
    {
        float c = (1 - abs(2 * l - 1)) * s;
        float H = h / 60;
        float x = c * (1 - abs(_mod(H, 2) - 1));
        float m = l - c / 2;
        _setPixelHCXM(pixel, H, c, x, m);
    }

    void Led::setPixelHSL(u_int16_t pixel, float h, float s, float l)
    {
        _setPixelHSL(pixel, _wrap360(h), _lim1(s), _lim1(l));
    }

    void Led::_setPixelHSI(u_int16_t pixel, float h, float s, float i)
    {
        float H = h / 60;
        float z = 1 - abs(_mod(H, 2) - 1);
        float c = (3.0 * i * s) / (1 + z);
        float x = c * z;
        float m = i * (1 - s);
        _setPixelHCXM(pixel, H, c, x, m);
    }

    void Led::setPixelHSI(u_int16_t pixel, float h, float s, float i)
    {
        _setPixelHSI(pixel, _wrap360(h), _lim1(s), _lim1(i));
    }

    void Led::getPixelRGB(u_int16_t pixel, float *r, float *g, float *b)
    {
        u_int16_t pixel_num = pixel * 3;
        *g = 1.0 * _led_pixels[pixel_num++] / 255;
        *r = 1.0 * _led_pixels[pixel_num++] / 255;
        *b = 1.0 * _led_pixels[pixel_num] / 255;
    }

    void Led::getPixelHSV(u_int16_t pixel, float *h, float *s, float *v)
    {
        float r;
        float g;
        float b;
        getPixelRGB(pixel, &r, &g, &b);
        float xmax = _max3(&r, &g, &b);
        *v = xmax;
        float xmin = _min3(&r, &g, &b);
        float c = xmax - xmin;
        if (c == 0)
            *h = 0;
        else if (xmax == r)
            *h = 60 * _mod((g - b) / c, 6);
        else if (xmax == g)
            *h = 60 * ((b - r) / c + 2);
        else
            *h = 60 * ((r - g) / c + 4);
        *s = (xmax == 0 ? 0 : c / xmax);
    }

    void Led::getPixelHSL(u_int16_t pixel, float *h, float *s, float *l)
    {
        float r;
        float g;
        float b;
        getPixelRGB(pixel, &r, &g, &b);
        float xmax = _max3(&r, &g, &b);
        float xmin = _min3(&r, &g, &b);
        float c = xmax - xmin;
        *l = (xmax + xmin) / 2;
        if (c == 0)
            *h = 0;
        else if (xmax == r)
            *h = 60 * _mod((g - b) / c, 6);
        else if (xmax == g)
            *h = 60 * ((b - r) / c + 2);
        else
            *h = 60 * ((r - g) / c + 4);
        *s = ((*l == 0 || *l == 1) ? 0 : c / (1 - abs(2 * xmax - c - 1)));
    }
}