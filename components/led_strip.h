#pragma once

#include <stdint.h>
#include <string.h>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"

namespace LED
{
    class Led
    {
    private:
        static bool _running;
        static rmt_channel_handle_t _led_channel;
        static rmt_encoder_handle_t _led_encoder;
        static rmt_transmit_config_t _tx_config;
        static u_int8_t _led_pixels[];
        static size_t _rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
        static esp_err_t _rmt_del_led_strip_encoder(rmt_encoder_t *encoder);
        static esp_err_t _rmt_led_strip_encoder_reset(rmt_encoder_t *encoder);
        static esp_err_t _rmt_new_led_strip_encoder(rmt_encoder_handle_t *ret_encoder);

        static void _setPixelRGB(u_int16_t pixel, float r, float g, float b);
        static void _setPixelHCXM(u_int16_t pixel, float H, float c, float x, float m);
        static void _setPixelHSV(u_int16_t pixel, float h, float s, float v);
        static void _setPixelHSL(u_int16_t pixel, float h, float s, float l);
        static void _setPixelHSI(u_int16_t pixel, float h, float s, float i);

    public:
        Led();
        typedef struct // I think C++ does not need typedef, but the program breaks without it
        {
            rmt_encoder_t base;
            rmt_encoder_t *bytes_encoder;
            rmt_encoder_t *copy_encoder;
            int state;
            rmt_symbol_word_t reset_code;
        } rmt_led_strip_encoder_t;

        /**
         * @brief Initialises RMT LED strip controller for WS2812
         *
         * @return
         *      - ESP_OK if initialised successfully
         */
        static esp_err_t init();

        /**
         * @brief Sends stored pixel data to LED strip
         *
         * @return
         *      - ESP_OK if sent successfully
         */

        static esp_err_t send();
        /**
         * @brief Gets state of LED strip
         *
         * @return
         *      - true if LED strip initialised
         */
        [[nodiscard]] bool ledState() { return _running; }

        /**
         * @brief Clears pixel data
         */
        static void clear();

        /**
         * @brief Sets RGB pixel data
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[in] r Red [0 - 1]
         * @param[in] g Green [0 - 1]
         * @param[in] b Blue [0 - 1]
         */
        static void setPixelRGB(u_int16_t pixel, float r, float g, float b);

        /**
         * @brief Sets HSV pixel data
         * @see setPixelHSI
         * @see setPixelHSL
         * @see setPixelHSV
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[in] h Hue [0 - 360)
         * @param[in] s Saturation [0 - 1]
         * @param[in] v Value [0 - 1]
         */
        static void setPixelHSV(u_int16_t pixel, float h, float s, float v);

        /**
         * @brief Sets HSL pixel data
         * @see setPixelHSI
         * @see setPixelHSV
         * @see setPixelRGB
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[in] h Hue [0 - 360)
         * @param[in] s Saturation [0 - 1]
         * @param[in] l Light [0 - 1]
         */
        static void setPixelHSL(u_int16_t pixel, float h, float s, float l);

        /**
         * @brief Sets HSI pixel data
         * @see setPixelHSL
         * @see setPixelHSV
         * @see setPixelRGB
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[in] h Hue [0 - 360)
         * @param[in] s Saturation [0 - 1]
         * @param[in] i Intensity [0 - 1]
         */
        static void setPixelHSI(u_int16_t pixel, float h, float s, float i);

        /**
         * @brief Gets RGB pixel data
         * @see setPixelRGB
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[out] r Red [0 - 1]
         * @param[out] g Green [0 - 1]
         * @param[out] b Blue [0 - 1]
         */
        static void getPixelRGB(u_int16_t pixel, float *r, float *g, float *b);

        /**
         * @brief Gets HSV pixel data
         * @see setPixelHSV
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[out] h Hue [0 - 360)
         * @param[out] s Saturation [0 - 1]
         * @param[out] v Value [0 - 1]
         */
        static void getPixelHSV(u_int16_t pixel, float *h, float *s, float *v);

        /**
         * @brief Gets HSL pixel data
         * @see setPixelHSL
         *
         * @param[in] p Pixel Number [0 - CONFIG_ESP_LED_NUMBERS)
         * @param[out] h Hue [0 - 360)
         * @param[out] s Saturation [0 - 1]
         * @param[out] l Lightness [0 - 1]
         */
        static void getPixelHSL(u_int16_t pixel, float *h, float *s, float *l);
    };
}
