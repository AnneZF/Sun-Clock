#include <string>
#include "esp_log.h"
#include "esp_sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "driver/rtc_io.h"

#include "wifi.h"
#include "sntp.h"
#include "led_strip.h"
#include "led_blink.h"
#include "ssd1306.h"

// #define test
#ifdef test
#define LEDS 48
#else
#define LEDS CONFIG_ESP_LED_NUMBERS
#endif

typedef enum
{
    SUNRISE_START = 0,
    WAKE_TIME = 1,
    SUNRISE_END = 2,
    SUNSET_START = 3,
    SUNSET_HOLD = 4,
    SUNSET_END = 5,
    SLEEP_TIME = 6,
    CALCULATE = 7,
} eventTime_t;

WIFI::WiFi::state_e wifiState{WIFI::WiFi::state_e::NOT_INITIALISED};
WIFI::WiFi WiFi;
SNTP::Sntp Sntp;
LED::Led Leds;
BLINK::led_blink Led;
SSD1306::oled oLed;

static TaskHandle_t tickHandle;
static TaskHandle_t scheduleHandle;

static void tick(void *pvParameter);
static void setColourRange(void (*setter)(u_int16_t, float, float, float), float v0, float v1, float v2, float r0 = 0, float r1 = 0, float r2 = 0, int pixelStart = 0, int pixelEnd = LEDS);
static void sunriseStart(int ms);
static void sunriseEnd(int ms);
static void sunsetStart(int ms);
static void sunsetEnd(int ms);
static void sleep(int ms);
static void eventScheduler(void *pvParameter);
static void setup();