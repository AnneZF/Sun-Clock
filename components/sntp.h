#pragma once

#include <ctime>
#include <chrono>
#include <iomanip>
#include <string>
#include <cmath>

#include "esp_log.h"

#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h"

namespace SNTP
{
    class Sntp : private WIFI::WiFi
    {
        static void callbackOnNtpUpdate(timeval *tv);

    public:
        typedef enum
        {
            SUNRISE_START = 0,
            WAKE_TIME = 1,
            SUNRISE_END = 2,
            SUNSET_START = 3,
            SUNSET_HOLD = 4,
            SUNSET_END = 5,
            SLEEP_TIME = 6,
            CALCULATE = 7, // recalculates wake time at 0200 - for transition in and out of dst
        } eventTime_t;

    private:
        static std::chrono::_V2::system_clock::time_point _lastUpdate;
        static bool _running;
        static u_int32_t _sync_interval;
        static double getUTToSunEvent(bool isSunrise, float zenith, int dayOfYear);

        static constexpr int wakeTime[7][3] = {{CONFIG_ESP_ALARM_0_0_0, CONFIG_ESP_ALARM_0_0_1, CONFIG_ESP_ALARM_0_0_2},
                                               {CONFIG_ESP_ALARM_1_0_0, CONFIG_ESP_ALARM_1_0_1, CONFIG_ESP_ALARM_1_0_2},
                                               {CONFIG_ESP_ALARM_2_0_0, CONFIG_ESP_ALARM_2_0_1, CONFIG_ESP_ALARM_2_0_2},
                                               {CONFIG_ESP_ALARM_3_0_0, CONFIG_ESP_ALARM_3_0_1, CONFIG_ESP_ALARM_3_0_2},
                                               {CONFIG_ESP_ALARM_4_0_0, CONFIG_ESP_ALARM_4_0_1, CONFIG_ESP_ALARM_4_0_2},
                                               {CONFIG_ESP_ALARM_5_0_0, CONFIG_ESP_ALARM_5_0_1, CONFIG_ESP_ALARM_5_0_2},
                                               {CONFIG_ESP_ALARM_6_0_0, CONFIG_ESP_ALARM_6_0_1, CONFIG_ESP_ALARM_6_0_2}};

        static constexpr int sleepTime[7][3] = {{CONFIG_ESP_ALARM_0_1_0, CONFIG_ESP_ALARM_0_1_1, CONFIG_ESP_ALARM_0_1_2},
                                                {CONFIG_ESP_ALARM_1_1_0, CONFIG_ESP_ALARM_1_1_1, CONFIG_ESP_ALARM_1_1_2},
                                                {CONFIG_ESP_ALARM_2_1_0, CONFIG_ESP_ALARM_2_1_1, CONFIG_ESP_ALARM_2_1_2},
                                                {CONFIG_ESP_ALARM_3_1_0, CONFIG_ESP_ALARM_3_1_1, CONFIG_ESP_ALARM_3_1_2},
                                                {CONFIG_ESP_ALARM_4_1_0, CONFIG_ESP_ALARM_4_1_1, CONFIG_ESP_ALARM_4_1_2},
                                                {CONFIG_ESP_ALARM_5_1_0, CONFIG_ESP_ALARM_5_1_1, CONFIG_ESP_ALARM_5_1_2},
                                                {CONFIG_ESP_ALARM_6_1_0, CONFIG_ESP_ALARM_6_1_1, CONFIG_ESP_ALARM_6_1_2}};

    public:
        Sntp() = default;
        ~Sntp() { esp_sntp_stop(); };

        static int timeTo[8];
        static int eventNow;

        /**
         * @brief Initialises SNTP
         *
         * @return
         *      - ESP_OK if initialised successfully
         */
        static esp_err_t init();

        /**
         * @brief Sets SNTP update interval
         *
         * @param[in] ms Update interval (ms)
         * @param[in] immediate Restarts SNTP if true
         * @return
         *      true if SNTP is running and interval can be set
         */
        static bool setUpdateInterval(u_int32_t ms, bool immediate = false);

        /**
         * @brief Gets SNTP state
         *
         * @return
         *      true if SNTP is running
         */

        [[nodiscard]] bool sntpState() { return _running; }

        [[nodiscard]] static const auto timePointNow();

        [[nodiscard]] static const auto timeSinceLastUpdate();

        [[nodiscard]] static const char *timeNowAscii();

        [[nodiscard]] static std::chrono::seconds epochSeconds();

        /**
         * @brief Gets time to sun event
         * @remarks see https://edwilliams.org/sunrise_sunset_algorithm.htm for calculations
         * @see getUTToSunEvent
         *
         * @param[in] isSunrise true if sunrise, false if Sunset
         * @param[in] isOfficial true (default) if Official sun event, else Astronomical sun event calculated
         * @param[in] isToday true (default) if today's sun event, else tommorrow's sun event calculated
         * @return
         *      '-1' if sun event does not happen on given day, else ms to event from now
         */
        [[nodiscard]] static int32_t msToSunEvent(bool isSunrise, bool isOfficial = true, bool isToday = true);

        /**
         * @brief Gets time to event
         *
         * @param[in] hours hours
         * @param[in] mins mins
         * @param[in] secs secs
         * @return
         *      ms to input time
         */
        [[nodiscard]] static int32_t msToLocTime(int hours = 0, int mins = 0, int secs = 0);

        [[nodiscard]] static int8_t getWDay();

        static void eventCalculator();
    };
}