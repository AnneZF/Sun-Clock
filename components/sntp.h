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

#define RAD(x) x *M_PI / 180.0
#define DEG(x) x * 180.0 / M_PI
#define LIM(x, y) (x >= y ? x - y : x < 0 ? x + y \
                                          : x);

namespace SNTP
{
    class Sntp : private WIFI::WiFi
    {
        static void callbackOnNtpUpdate(timeval *tv);

    private:
        static std::chrono::_V2::system_clock::time_point _lastUpdate;
        static bool _running;
        static u_int32_t _sync_interval;
        double getUTToSunEvent(bool isSunrise, float zenith, int dayOfYear);

    public:
        Sntp() = default;
        ~Sntp() { esp_sntp_stop(); };

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
         *
         * @param[in] isSunrise true if sunrise, false if Sunset
         * @param[in] isOfficial true (default) if Official sun event, else Astronomical sun event calculated
         * @param[in] isToday true (default) if today's sun event, else tommorrow's sun event calculated
         * @return
         *      '-1' if sun event does not happen on given day, else ms to event from now
         */
        int32_t msToSunEvent(bool isSunrise, bool isOfficial = true, bool isToday = true);

        /**
         * @brief Gets time to event
         *
         * @param[in] hours hours
         * @param[in] mins mins
         * @param[in] secs secs
         * @return
         *      ms to input time
         */
        int32_t msToLocTime(int hours = 0, int mins = 0, int secs = 0);
    };
}