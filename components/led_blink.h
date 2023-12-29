#pragma once

#include "driver/gpio.h"
#include "esp_log.h"

namespace BLINK
{
    class led_blink
    {
    private:
        static bool _led_state;
        static bool _running;

    public:
        led_blink();
        static bool ledState() { return _running; }
        static esp_err_t init();
        static void blink();
    };
}