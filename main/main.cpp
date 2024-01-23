#include "main.h"

void tick(void *pvParameter)
{
    TickType_t startTick;
    while (true)
    {
        startTick = xTaskGetTickCount();
        if (CONFIG_ESP_ONBOARD_LED)
            Led.blink();
        if (CONFIG_ESP_OLED)
        {
            oLed.drawTime(Sntp.timeNowAscii());
            oLed.refresh();
        }
        xTaskDelayUntil(&startTick, pdMS_TO_TICKS(1000));
    }
}

void setColourRange(void (*setter)(u_int16_t, float, float, float), float v0, float v1, float v2, float r0, float r1, float r2, int pixelStart, int pixelEnd)
{
    float a0 = v0 - r0;
    float dA0 = (r0 * 2) / (pixelEnd - pixelStart);
    float a1 = v1 - r1;
    float dA1 = (r1 * 2) / (pixelEnd - pixelStart);
    float a2 = v2 - r2;
    float dA2 = (r2 * 2) / (pixelEnd - pixelStart);
    for (int p = pixelStart; p < pixelEnd; p++)
    {
        setter(p, a0, a1, a2);
        a0 += dA0;
        a1 += dA1;
        a2 += dA2;
    }
}

void sunriseStart(int ms)
{
    TickType_t startTick;
    float d = 1.0 * ms / CONFIG_ESP_LED_REFRESH_MS;
    float h = 240;
    float dH = 180.0 / d; // blue to yellow
    float l = 0.001;
    float dL = 0.949 / d;
    float a = 60;
    float dA = -30.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTick = xTaskGetTickCount();
        setColourRange(&Leds.setPixelHSL, h, 1, l, a);
        Leds.send();
        h += dH;
        l += dL;
        a += dA;
        vTaskDelayUntil(&startTick, pdMS_TO_TICKS(CONFIG_ESP_LED_REFRESH_MS));
    }

    setColourRange(&Leds.setPixelHSL, 0, 1, 1);
    Leds.send();
}

void sunriseEnd(int ms)
{
    TickType_t startTick;
    float d = 1.0 * ms / CONFIG_ESP_LED_REFRESH_MS;
    float v = 0.95;
    float dV = -0.95 / d;
    for (int i = 0; i < d; i++)
    {
        startTick = xTaskGetTickCount();
        setColourRange(&Leds.setPixelHSL, 0, v, v);
        Leds.send();
        v += dV;
        vTaskDelayUntil(&startTick, pdMS_TO_TICKS(CONFIG_ESP_LED_REFRESH_MS));
    }

    Leds.clear();
    Leds.send();
}

void sunsetStart(int ms)
{
    TickType_t startTick;
    float d = 1.0 * ms / CONFIG_ESP_LED_REFRESH_MS;
    float h = 60;
    float dH = 300.0 / d; // yellow to blue
    float l = 0.001;
    float dL = 0.949 / d;
    float a = 30;
    float dA = 45.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTick = xTaskGetTickCount();
        setColourRange(&Leds.setPixelHSL, h, 1, l, a);
        Leds.send();
        h += dH;
        l += dL;
        a += dA;
        vTaskDelayUntil(&startTick, pdMS_TO_TICKS(CONFIG_ESP_LED_REFRESH_MS));
    }
}

void sunsetEnd(int ms)
{
    TickType_t startTick;
    float d = 1.0 * ms / CONFIG_ESP_LED_REFRESH_MS;
    float h = 0;
    float l = 0.949;
    float a = 75;
    float dA = -45.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTick = xTaskGetTickCount();
        setColourRange(&Leds.setPixelHSL, h, 1, l, a);
        Leds.send();
        l = 1.0 * (sin(M_PI_2 * (i * CONFIG_ESP_LED_REFRESH_MS / ms + 1)) * (3 + sin(M_PI * i * CONFIG_ESP_LED_REFRESH_MS / 1000 + M_PI_2))) / 4;
        a += dA;
        vTaskDelayUntil(&startTick, pdMS_TO_TICKS(CONFIG_ESP_LED_REFRESH_MS));
    }

    Leds.clear();
    Leds.send();
}

void sleep(int ms)
{
    esp_sleep_enable_timer_wakeup(static_cast<u_int64_t>(ms) * 1000);
    WiFi.stop();
    if (CONFIG_ESP_OLED)
        oLed.power_down();
    esp_deep_sleep_start();
}

void eventScheduler(void *pvParameter)
{
    TickType_t startTick;
    switch (Sntp.eventNow)
    {
    case SUNRISE_START:
        ESP_LOGI("Event Scheduler", "Sleeping till Sunrise...");
        sleep(Sntp.timeTo[SUNRISE_START]);
        break;

    case WAKE_TIME:
        Sntp.timeTo[SUNRISE_START] = 0;
        Sntp.eventNow = SUNRISE_START;
        break;

    case SUNRISE_END:
        if (Sntp.timeTo[SUNRISE_END] > 5000)
        {
            Sntp.timeTo[SUNRISE_START] = 0;
            Sntp.timeTo[WAKE_TIME] = 5000;
            Sntp.eventNow = SUNRISE_START;
        }
        else
            Sntp.eventNow = SUNSET_START;
        break;

    case SUNSET_START:
        break;

    case SUNSET_HOLD:
        Sntp.timeTo[SUNSET_START] = 0;
        Sntp.eventNow = SUNSET_START;
        break;

    case SUNSET_END:
        Sntp.timeTo[SUNSET_START] = 0;
        Sntp.timeTo[SUNSET_HOLD] = 5000;
        Sntp.eventNow = SUNSET_START;
        break;

    case SLEEP_TIME:
        if (Sntp.timeTo[SLEEP_TIME] > 10000)
        {
            ESP_LOGI("Event Scheduler", "Sunset Start");
            if (CONFIG_ESP_LED_STRIP)
                sunsetStart(5000);
            Sntp.timeTo[SUNSET_END] = 5000;
            Sntp.eventNow = SUNSET_END;
        }
        break;

    case CALCULATE:
        Sntp.eventNow = SLEEP_TIME;
        break;

    default:
        ESP_LOGE("Event Scheduler", "Cannot find correct time...");
        break;
    }

    while (true)
    {
        switch (Sntp.eventNow)
        {
        case SUNRISE_START:
            ESP_LOGI("Scheduler", "Sunrise Start");
            if (CONFIG_ESP_LED_STRIP)
                sunriseStart(Sntp.timeTo[WAKE_TIME] - Sntp.timeTo[SUNRISE_START]);

        case WAKE_TIME:

        case SUNRISE_END:
            ESP_LOGI("Scheduler", "Sunrise End");
            if (CONFIG_ESP_LED_STRIP)
                sunriseEnd(Sntp.timeTo[SUNRISE_END] - Sntp.timeTo[WAKE_TIME]);

        case SUNSET_START:
            if (Sntp.timeTo[SUNSET_START] > 0)
            {
                ESP_LOGI("Scheduler", "Waiting for Sunset Start");
                Sntp.eventCalculator();
                startTick = xTaskGetTickCount();
                xTaskDelayUntil(&startTick, pdMS_TO_TICKS(Sntp.timeTo[SUNSET_START]));
            }
            ESP_LOGI("Scheduler", "Sunset Start");
            if (CONFIG_ESP_LED_STRIP)
                sunsetStart(Sntp.timeTo[SUNSET_HOLD] - Sntp.timeTo[SUNSET_START]);

        case SUNSET_HOLD:
            if (Sntp.timeTo[SUNSET_END] > 0)
            {
                ESP_LOGI("Scheduler", "Sunset Hold");
                Sntp.eventCalculator();
                startTick = xTaskGetTickCount();
                xTaskDelayUntil(&startTick, pdMS_TO_TICKS(Sntp.timeTo[SUNSET_END]));
            }

        case SUNSET_END:
            ESP_LOGI("Scheduler", "Sunset End");
            if (CONFIG_ESP_LED_STRIP)
                sunsetEnd(Sntp.timeTo[SLEEP_TIME] - Sntp.timeTo[SUNSET_END]);

        case SLEEP_TIME:
            ESP_LOGI("Scheduler", "Sleeping till Calculation Time...");
            Sntp.eventCalculator();
            sleep(Sntp.timeTo[CALCULATE]);

        case CALCULATE:
        default:
            vTaskDelay(60 * 60 * 1000);
        }
    }
}

void setup(void)
{
    esp_event_loop_create_default();
    if (CONFIG_ESP_ONBOARD_LED && !Led.ledState())
    {
        Led.init();
    }
    if (CONFIG_ESP_LED_STRIP && !Leds.ledState())
    {
        Leds.init();
    }
    if (CONFIG_ESP_OLED && !oLed.oled_state())
    {
        oLed.init();
    }
    nvs_flash_init();
    ESP_LOGI("Main", "Initialising WiFi...");
    WiFi.init();
    while (!(WIFI::WiFi::state_e::CONNECTED == wifiState ||
             WIFI::WiFi::state_e::DISCONNECTED == wifiState))
        wifiState = WiFi.GetState();
    if (WIFI::WiFi::state_e::CONNECTED == wifiState)
    {
        if (!Sntp.sntpState())
        {
            ESP_LOGI("Main", "Initialising SNTP...");
            Sntp.init();
        }
        while (!Sntp.sntpState())
            ;
    }
    else
    {
        esp_restart();
    }

#if CONFIG_IDF_TARGET_ESP32
    // Isolate GPIO12 pin from external circuits. This is needed for modules which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER) to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#endif
}

extern "C" void app_main(void)
{
    setup();
    xTaskCreate(eventScheduler, "schedule", 2048, nullptr, 10, &scheduleHandle);
    // ESP_LOGI("scheduler: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(scheduleHandle));
    vTaskDelay(100); // to avoid blinking if the next task is sleep
    xTaskCreate(tick, "tick", 2048, nullptr, 10, &tickHandle);
    // ESP_LOGI("tick: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(tickHandle));
}