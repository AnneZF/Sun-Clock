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

// #define test
#ifdef test
#define LEDS 48
#else
#define LEDS CONFIG_ESP_LED_NUMBERS
#endif

typedef enum
{
    SUNRISE_START = 0,
    SUNRISE_END = 1,
    SUNSET_START = 2,
    SUNSET_HOLD = 3,
    SUNSET_END = 4,
    SLEEP = 5
} eventTime_t;

WIFI::WiFi::state_e wifiState{WIFI::WiFi::state_e::NOT_INITIALISED};
WIFI::WiFi WiFi;
SNTP::Sntp Sntp;
LED::Led Leds;
BLINK::led_blink Led;

static TaskHandle_t tickHandle;
static TaskHandle_t scheduleHandle;

int wakeTime[7][3] = {{CONFIG_ESP_ALARM_0_0_0, CONFIG_ESP_ALARM_0_0_1, CONFIG_ESP_ALARM_0_0_2},
                      {CONFIG_ESP_ALARM_1_0_0, CONFIG_ESP_ALARM_1_0_1, CONFIG_ESP_ALARM_1_0_2},
                      {CONFIG_ESP_ALARM_2_0_0, CONFIG_ESP_ALARM_2_0_1, CONFIG_ESP_ALARM_2_0_2},
                      {CONFIG_ESP_ALARM_3_0_0, CONFIG_ESP_ALARM_3_0_1, CONFIG_ESP_ALARM_3_0_2},
                      {CONFIG_ESP_ALARM_4_0_0, CONFIG_ESP_ALARM_4_0_1, CONFIG_ESP_ALARM_4_0_2},
                      {CONFIG_ESP_ALARM_5_0_0, CONFIG_ESP_ALARM_5_0_1, CONFIG_ESP_ALARM_5_0_2},
                      {CONFIG_ESP_ALARM_6_0_0, CONFIG_ESP_ALARM_6_0_1, CONFIG_ESP_ALARM_6_0_2}};

int sleepTime[7][3] = {{CONFIG_ESP_ALARM_0_1_0, CONFIG_ESP_ALARM_0_1_1, CONFIG_ESP_ALARM_0_1_2},
                       {CONFIG_ESP_ALARM_1_1_0, CONFIG_ESP_ALARM_1_1_1, CONFIG_ESP_ALARM_1_1_2},
                       {CONFIG_ESP_ALARM_2_1_0, CONFIG_ESP_ALARM_2_1_1, CONFIG_ESP_ALARM_2_1_2},
                       {CONFIG_ESP_ALARM_3_1_0, CONFIG_ESP_ALARM_3_1_1, CONFIG_ESP_ALARM_3_1_2},
                       {CONFIG_ESP_ALARM_4_1_0, CONFIG_ESP_ALARM_4_1_1, CONFIG_ESP_ALARM_4_1_2},
                       {CONFIG_ESP_ALARM_5_1_0, CONFIG_ESP_ALARM_5_1_1, CONFIG_ESP_ALARM_5_1_2},
                       {CONFIG_ESP_ALARM_6_1_0, CONFIG_ESP_ALARM_6_1_1, CONFIG_ESP_ALARM_6_1_2}};

static void tick(void *pvParameter);
static void setColourRange(void (*setter)(u_int16_t, float, float, float), float v0, float v1, float v2, float r0 = 0, float r1 = 0, float r2 = 0, int pixelStart = 0, int pixelEnd = LEDS);
static void sunriseStart(int ms);
static void sunriseEnd(int ms);
static void sunsetStart(int ms);
static void sunsetEnd(int ms);
static int eventCalculator(int (&timeTo)[6]);
static void eventScheduler(void *pvParameter);
static void setup();