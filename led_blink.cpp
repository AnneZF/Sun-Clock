#include "led_blink.h"

namespace BLINK
{
    bool led_blink::_led_state{false};
    bool led_blink::_running{false};

    led_blink::led_blink(){};

    esp_err_t led_blink::init()
    {
        esp_err_t state{ESP_OK};
        state = gpio_reset_pin(static_cast<gpio_num_t>(CONFIG_ESP_ONBOARD_LED_GPIO_NUM));
        if (ESP_OK == state)
            state = gpio_set_direction(static_cast<gpio_num_t>(CONFIG_ESP_ONBOARD_LED_GPIO_NUM), GPIO_MODE_OUTPUT);
        if (ESP_OK == state)
        {
            ESP_LOGI("LED", "Blink Initialised");
            _running = true;
        }
        return state;
    }

    void led_blink::blink()
    {
        _led_state = !_led_state;
        gpio_set_level(static_cast<gpio_num_t>(CONFIG_ESP_ONBOARD_LED_GPIO_NUM), _led_state);
    }
}