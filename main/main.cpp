#include "main.h"

TaskHandle_t tickHandle;
TaskHandle_t scheduleHandle;
// Main Clock;

// void Main::tick(void) {
//     TickType_t start_time = xTaskGetTickCount();
//     ESP_LOGI("Main Update", "%s", Sntp.timeNowAscii());
//     vTaskDelayUntil(&start_time, pdMS_TO_TICKS(1000));

//     // ESP_LOGI("Sunrise, Official", "%li", Sntp.msToSunEvent(true));
//     // ESP_LOGI("Sunrise, Astronomical", "%li", Sntp.msToSunEvent(true, 108));
//     // ESP_LOGI("Sunset, Official", "%li", Sntp.msToSunEvent(false));
//     // ESP_LOGI("Sunset, Astronomical", "%li", Sntp.msToSunEvent(false, 108));
// }

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

void tick(void *pvParameter)
{
    TickType_t startTime;
    while (true)
    {
        startTime = xTaskGetTickCount();
        ESP_LOGI("Main Update", "%s", Sntp.timeNowAscii());
        xTaskDelayUntil(&startTime, pdMS_TO_TICKS(1000));
    }
}

void setColourRange(void (*setter)(u_int16_t, float, float, float), float v0, float v1, float v2, float r0 = 0, float r1 = 0, float r2 = 0, int pixelStart = 0, int pixelEnd = LEDS)
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
    TickType_t startTime;
    int dT = 100;
    float d = 1.0 * ms / dT;
    float h = 240;
    float dH = 180.0 / d; // blue to yellow
    float l = 0.001;
    float dL = 0.999 / d;
    float a = 60;
    float dA = -30.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTime = xTaskGetTickCount();
        setColourRange(&Led.setPixelHSL, h, 1, l, a);
        Led.send();
        h += dH;
        l += dL;
        a += dA;
        vTaskDelayUntil(&startTime, pdMS_TO_TICKS(dT));
    }

    setColourRange(&Led.setPixelHSL, 0, 1, 1);
    Led.send();
}

void sunriseEnd(int ms)
{
    TickType_t startTime;
    int dT = 100;
    float d = 1.0 * ms / dT;
    float v = 1;
    float dV = -0.999 / d;
    for (int i = 0; i < d; i++)
    {
        startTime = xTaskGetTickCount();
        setColourRange(&Led.setPixelHSL, 0, v, v);
        Led.send();
        v -= dV;
        vTaskDelayUntil(&startTime, pdMS_TO_TICKS(dT));
    }

    Led.clear();
    Led.send();
}

void sunsetStart(int ms)
{
    TickType_t startTime;
    int dT = 100;
    float d = 1.0 * ms / dT;
    float h = 420;
    float dH = -180.0 / d; // yellow to blue
    float l = 0.001;
    float dL = 0.999 / d;
    float a = 30;
    float dA = 30.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTime = xTaskGetTickCount();
        setColourRange(&Led.setPixelHSL, h, 1, l, a);
        Led.send();
        h += dH;
        l += dL;
        a += dA;
        vTaskDelayUntil(&startTime, pdMS_TO_TICKS(dT));
    }

    setColourRange(&Led.setPixelHSL, 0, 1, 1);
    Led.send();
}

void sunsetEnd(int ms)
{
    TickType_t startTime;
    float t2 = ms / 1000;
    int dT = 100;
    float d = 1.0 * ms / dT;
    float h = 0;
    float l = 1;
    float a = 180;
    float dA = -120.0 / d;
    for (int i = 0; i < d; i++)
    {
        startTime = xTaskGetTickCount();
        setColourRange(&Led.setPixelHSL, h, 1, l, a);
        Led.send();
        l = (sin(std::numbers::pi * i * dT / t2 + std::numbers::pi / 2) * (3 + sin(2 * std::numbers::pi * i * dT + std::numbers::pi / 2))) / 4;
        a += dA;
        vTaskDelayUntil(&startTime, pdMS_TO_TICKS(dT));
    }

    Led.clear();
    Led.send();
}

void eventScheduler(void *pvParameter)
{
    TickType_t startTime;
    while (true)
    {
        startTime = xTaskGetTickCount();
        ESP_LOGI("Main", "scheduler");
        sunsetStart(10000);
        ESP_LOGI("Main", "scheduler");
        xTaskDelayUntil(&startTime, pdMS_TO_TICKS(15000));
        ESP_LOGI("Main", "scheduler");
        startTime = xTaskGetTickCount();
        ESP_LOGI("Main", "scheduler");
        sunsetEnd(10000);
        xTaskDelayUntil(&startTime, pdMS_TO_TICKS(15000));
    }
}

void setup(void)
{
    esp_event_loop_create_default();
    if (!Led.ledState())
    {
        ESP_LOGI("Main", "Initialising LED...");
        Led.init();
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
}

extern "C" void app_main(void)
{
    setup();
    xTaskCreate(tick, "tick", 2048, nullptr, 10, &tickHandle);
    ESP_LOGI("tick: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(tickHandle));
    xTaskCreate(eventScheduler, "schedule", 4096, nullptr, 10, &scheduleHandle);
    vTaskDelay(1000);
    ESP_LOGI("scheduler: Stack High Water Mark", "%i", uxTaskGetStackHighWaterMark(scheduleHandle));
}
