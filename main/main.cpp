#include "main.h"

void tick(void *pvParameter)
{
    TickType_t startTick;
    while (true)
    {
        startTick = xTaskGetTickCount();
        // ESP_LOGI("Tick", "%s", Sntp.timeNowAscii());
        Led.blink();
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
    float dL = 0.999 / d;
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
    float v = 1;
    float dV = -0.999 / d;
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

    // setColourRange(&Leds.setPixelHSL, 0, 1, 1);
    // Leds.send();
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
        l = 1.0 * (sin(M_PI * i * CONFIG_ESP_LED_REFRESH_MS / 2 / ms + M_PI / 2) * (3 + sin(2.0 * M_PI * i * CONFIG_ESP_LED_REFRESH_MS / 1000 + M_PI / 2))) / 4;
        a += dA;
        vTaskDelayUntil(&startTick, pdMS_TO_TICKS(CONFIG_ESP_LED_REFRESH_MS));
    }

    Leds.clear();
    Leds.send();
}

int eventCalculator(int (&timeTo)[6])
{
    int wday = Sntp.getWDay();
    timeTo[0] = Sntp.msToLocTime(wakeTime[wday][0], wakeTime[wday][1], wakeTime[wday][2]);
    timeTo[1] = Sntp.msToSunEvent(true);
    timeTo[2] = Sntp.msToSunEvent(false);
    timeTo[3] = Sntp.msToSunEvent(false, false);
    timeTo[4] = Sntp.msToLocTime(sleepTime[wday][0], sleepTime[wday][1], sleepTime[wday][2]);
    timeTo[5] = Sntp.msToLocTime(2);

    if (timeTo[1] - timeTo[0] < 30 * 60 * 1000)
        timeTo[1] = timeTo[0];

    timeTo[0] -= 30 * 60 * 1000;

    if (timeTo[3] == -1)
        timeTo[3] = timeTo[2] + 30 * 60 * 1000;

    timeTo[4] -= 30 * 60 * 1000;

    if (timeTo[5] < 0)
        timeTo[5] = Sntp.msToLocTime(26);

    ESP_LOGI("Calculated Schedule", "\n\nSunrise Start:\t%- 10i\nSunrise End:\t%- 10i\nSunset Start:\t%- 10i\nSunset Hold:\t%- 10i\nSunset End:\t%- 10i\nSleep:\t\t%- 10i", timeTo[0], timeTo[1], timeTo[2], timeTo[3], timeTo[4], timeTo[5]);

    for (int i = 0; i < 6; i++)
    {
        if (timeTo[i] > 0)
        {
            if (i == 0 && timeTo[5] < timeTo[0])
                return 5;
            return i;
        }
    }
    return 6;
}

void eventScheduler(void *pvParameter)
{
    TickType_t startTick = xTaskGetTickCount();

    int timeTo[6];
    char eventNow = eventCalculator(timeTo);

    switch (eventNow)
    {
    case SUNRISE_START:
        ESP_LOGI("Event Scheduler", "Sleeping till Sunrise...");
        esp_sleep_enable_timer_wakeup(timeTo[0] * 1000);
        esp_wifi_stop();
        esp_deep_sleep_start();
        break;

    case SUNRISE_END:
        timeTo[0] = 0;
        eventNow = SUNRISE_START;
        break;

    case SUNSET_START:
        xTaskDelayUntil(&startTick, pdMS_TO_TICKS(timeTo[2]));
        break;

    case SUNSET_HOLD:
        timeTo[2] = 0;
        eventNow = SUNSET_START;
        break;

    case SUNSET_END:
        timeTo[2] = 0;
        timeTo[3] = 5000;
        eventNow = SUNSET_START;
        break;

    case SLEEP:
        if (timeTo[4] > -29 * 60 * 1000 - 5000)
        {
            timeTo[2] = 0;
            timeTo[3] = 5000;
            eventNow = SUNSET_START;
        }
        else
        {
            ESP_LOGI("Event Scheduler", "Sleeping till Calculation Time...");
            esp_sleep_enable_timer_wakeup(timeTo[5] * 1000);
            esp_wifi_stop();
            esp_deep_sleep_start();
        }
        break;

    default:
        ESP_LOGE("Event Scheduler", "Cannot Find Correct Time. Restarting...");
        esp_restart();
        break;
    }

    while (true)
    {
        switch (eventNow)
        {
        case SUNRISE_START:
            ESP_LOGI("Scheduler", "Sunrise Start");
            sunriseStart(timeTo[1] - timeTo[0]);

        case SUNRISE_END:
            ESP_LOGI("Scheduler", "Sunrise End");
            sunriseEnd(30 * 60 * 1000);
            xTaskDelayUntil(&startTick, pdMS_TO_TICKS(timeTo[2]));

        case SUNSET_START:
            ESP_LOGI("Scheduler", "Sunset Start");
            sunsetStart(timeTo[3] - timeTo[2]);

        case SUNSET_HOLD:
            ESP_LOGI("Scheduler", "Sunset Hold");
            xTaskDelayUntil(&startTick, pdMS_TO_TICKS(timeTo[4]));

        case SUNSET_END:
            ESP_LOGI("Scheduler", "Sunset End");
            sunsetEnd(30 * 60 * 1000);
            ESP_LOGI("Scheduler", "Sleeping till Calculation Time...");
            esp_sleep_enable_timer_wakeup((timeTo[5] - timeTo[4] - 30 * 60 * 1000) * 1000);
            esp_wifi_stop();
            esp_deep_sleep_start();

        case SLEEP:
            break;
            //     ESP_LOGI("Scheduler", "Calculating times...");
            //     startTick = xTaskGetTickCount();
            //     eventNow = eventCalculator(timeTo);

            //     ESP_LOGI("Scheduler", "Goodnight!");
            //     esp_sleep_enable_timer_wakeup(timeTo[0] * 1000);
            //     esp_wifi_stop();
            //     esp_deep_sleep_start();
        }
    }
}

void setup(void)
{
    esp_event_loop_create_default();
    if (!Led.ledState())
    {
        Led.init();
    }
    if (!Leds.ledState())
    {
        ESP_LOGI("Main", "Initialising LED...");
        Leds.init();
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
    xTaskCreate(tick, "tick", 2048, nullptr, 10, &tickHandle);
    // ESP_LOGI("tick: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(tickHandle));
    xTaskCreate(eventScheduler, "schedule", 2048, nullptr, 10, &scheduleHandle);
    // ESP_LOGI("scheduler: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(scheduleHandle));
}
