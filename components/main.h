#include <string>
// #include <cmath>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "sntp.h"
#include "led_strip.h"

#define test
#ifdef test 
#define LEDS 10
#else 
#define LEDS CONFIG_ESP_LED_NUMBERS
#endif
//  class Main final
//  {
//      private:
//      public:
//          // TaskFunction_t tick(void *pvParameter);
//          void setup(void);

//         WIFI::WiFi::state_e wifiState { WIFI::WiFi::state_e::NOT_INITIALISED };
//         WIFI::WiFi WiFi;
//         SNTP::Sntp Sntp;
// };

WIFI::WiFi::state_e wifiState{WIFI::WiFi::state_e::NOT_INITIALISED};
WIFI::WiFi WiFi;
SNTP::Sntp Sntp;
LED::Led Led;
