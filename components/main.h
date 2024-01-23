#include <string>
#include "esp_log.h"
#include "esp_sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "driver/rtc_io.h"

#include "wifi.h"
#include "sntp.h"
#include "led_blink.h"
#include "led_strip.h"
#include "ssd1306.h"

// #define test
#ifdef test
#define LEDS 12
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
BLINK::led_blink Led;
LED::Led Leds;
SSD1306::oled oLed;

static TaskHandle_t tickHandle;
static TaskHandle_t scheduleHandle;

/**
 * @brief Task that runs every 1000ms
 */
static void tick(void *pvParameter);

/**
 * @brief Sets LED Strip
 *
 * @param[in] setter choose from:
 *  - setPixelRGB(u_int16_t pixel, float r, float g, float b)
 *  - setPixelHSV(u_int16_t pixel, float h, float s, float v)
 *  - setPixelHSL(u_int16_t pixel, float h, float s, float l)
 *  - setPixelHSI(u_int16_t pixel, float h, float s, float i)
 * @param[in] v0 first initial value to be input into setter (r/h)
 * @param[in] v1 second initial value to be input into setter (g/s)
 * @param[in] v2 third initial value to be input into setter (b/v/l/i)
 * @param[in] r0 gradient of change for first value across pixels (r/h)
 * @param[in] r1 gradient of change for second value across pixels (g/s)
 * @param[in] r2 gradient of change for third value across pixels (b/v/l/i)
 * @param[in] pixelStart first pixel
 * @param[in] pixelEnd last pixel
 */
static void setColourRange(void (*setter)(u_int16_t, float, float, float), float v0, float v1, float v2, float r0 = 0, float r1 = 0, float r2 = 0, int pixelStart = 0, int pixelEnd = LEDS);

/**
 * @brief LED Strip at end of Sunrise
 *
 * @param[in] ms Duration of event in ms
 */
static void sunriseStart(int ms);

/**
 * @brief LED Strip at end of Sunrise
 *
 * @param[in] ms Duration of event in ms
 */
static void sunriseEnd(int ms);

/**
 * @brief LED Strip at start of Sunset
 *
 * @param[in] ms Duration of event in ms
 */
static void sunsetStart(int ms);

/**
 * @brief LED Strip at end of Sunset
 *
 * @param[in] ms Duration of event in ms
 */
static void sunsetEnd(int ms);

/**
 * @brief Turns off peripherals and puts ESP32 to sleep
 *
 * @param[in] ms Duration of event in ms
 */
static void sleep(int ms);

/**
 * @brief Schedules when events are run
 */
static void eventScheduler(void *pvParameter);

/**
 * @brief Sets up ESP32 and relevant peripherals
 * Configure in ESP settings before use!!
 */
static void setup();