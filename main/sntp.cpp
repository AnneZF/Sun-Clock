#include "sntp.h"

namespace SNTP
{
    std::chrono::_V2::system_clock::time_point Sntp::_lastUpdate{};
    bool Sntp::_running{false};

    int Sntp::timeTo[8];
    int Sntp::eventNow;

    void Sntp::callbackOnNtpUpdate(timeval *tv)
    {
        _lastUpdate = std::chrono::system_clock::now();
        eventCalculator();
    }

    esp_err_t Sntp::init()
    {
        if (!_running)
        {
            if (state_e::CONNECTED != WiFi::GetState())
            {
                ESP_LOGE("SNTP", "Initialisation Failed. WiFi not Connected.");
                return ESP_FAIL;
            }

            setenv("TZ", CONFIG_ESP_TIMEZONE, 1);
            tzset();

            esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
            esp_sntp_setservername(0, "time.google.com");
            esp_sntp_setservername(1, "pool.ntp.com");

            sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

            sntp_set_time_sync_notification_cb(&callbackOnNtpUpdate);
            sntp_set_sync_interval(CONFIG_ESP_SNTP_UPDATE_INTERVAL * 60 * 60 * 1000); // in ms, min 15 s

            esp_sntp_init();

            ESP_LOGI("SNTP", "Initialised");

            while (SNTP_SYNC_STATUS_COMPLETED != sntp_get_sync_status())
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }

            _running = true;

            ESP_LOGI("SNTP", "Synced");
        }
        return _running ? ESP_OK : ESP_FAIL;
    }

    bool Sntp::setUpdateInterval(u_int32_t ms, const bool immediate)
    {
        if (_running)
        {
            sntp_set_sync_interval(ms);
            _sync_interval = ms;
            if (immediate)
            {
                sntp_restart();
            }
            return true;
        }
        return false;
    }

    [[nodiscard]] const auto Sntp::timePointNow()
    {
        return std::chrono::system_clock::now();
    }

    [[nodiscard]] const auto Sntp::timeSinceLastUpdate()
    {
        return timePointNow() - _lastUpdate;
    }

    [[nodiscard]] std::chrono::seconds Sntp::epochSeconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(timePointNow().time_since_epoch());
    }

    double RAD(double x)
    {
        return x * M_PI / 180.0;
    }

    double DEG(double x)
    {
        return x * 180.0 / M_PI;
    }

    double LIM(double x, double y)
    {
        while (x > y)
            x -= y;
        while (x < 0)
            x += y;
        return x;
    }

    [[nodiscard]] const char *Sntp::timeNowAscii()
    {
        const std::time_t timeNow{std::chrono::system_clock::to_time_t(timePointNow())};
        return std::asctime(std::localtime(&timeNow));
    }

    [[nodiscard]] double Sntp::getUTToSunEvent(bool isSunrise, float zenith, int dayOfYear)
    {
        double longInHour = std::stod(CONFIG_ESP_LONGITUDE) / 15.0;
        double time = dayOfYear + ((isSunrise ? 6.0 : 18.0) - longInHour) / 24.0;
        double sunMeanAnom = (0.9856 * time) - 3.289;
        double sunLong = LIM(sunMeanAnom + 1.916 * sin(RAD(sunMeanAnom)) + 0.02 * sin(RAD(2 * sunMeanAnom)) + 282.634, 360.0);
        double sunRA = LIM(DEG(atan(0.91764 * tan(RAD(sunLong)))), 360.0);
        sunRA = (sunRA + (floor(sunLong / 90.0) * 90.0 - floor(sunRA / 90.0) * 90.0)) / 15.0;
        double sinDec = 0.39782 * sin(RAD(sunLong));
        double cosDec = cos(asin(sinDec));
        double cosH = (cos(RAD(zenith)) - sinDec * sin(RAD(std::stod(CONFIG_ESP_LATITUDE)))) / (cosDec * cos(RAD(std::stod(CONFIG_ESP_LATITUDE))));

        if (cosH > 1 || cosH < -1)
            return -1; // >1 -> never rise, <-1 -> never set

        return LIM((isSunrise ? (360.0 - DEG(acos(cosH))) : DEG(acos(cosH))) / 15.0 + sunRA - 0.06571 * time - 6.622 - longInHour, 24.0);
    }

    [[nodiscard]] int32_t Sntp::msToSunEvent(bool isSunrise, bool isOfficial, bool isToday)
    {
        std::time_t timeNow{std::chrono::system_clock::to_time_t(timePointNow())};
        std::tm *timeNow_gm = std::gmtime(&timeNow);
        double UT = getUTToSunEvent(isSunrise, (isOfficial ? 90.833 : 108.0), timeNow_gm->tm_yday + (isToday ? 0 : 1));

        if (UT == -1)
            return -1;

        return floor((((UT + (isToday ? 0 : 24) - timeNow_gm->tm_hour) * 60 - timeNow_gm->tm_min) * 60 - timeNow_gm->tm_sec) * 1000);
    }

    [[nodiscard]] int32_t Sntp::msToLocTime(int hours, int mins, int secs)
    {
        std::time_t timeNow{std::chrono::system_clock::to_time_t(timePointNow())};
        std::tm *timeNow_loc = std::localtime(&timeNow);
        return (((hours - timeNow_loc->tm_hour) * 60 + mins - timeNow_loc->tm_min) * 60 + secs - timeNow_loc->tm_sec) * 1000;
    }

    [[nodiscard]] int8_t Sntp::getWDay()
    {
        std::time_t timeNow{std::chrono::system_clock::to_time_t(timePointNow())};
        return std::localtime(&timeNow)->tm_wday;
    }

    void Sntp::eventCalculator()
    {
        int wday = getWDay();
        timeTo[WAKE_TIME] = msToLocTime(wakeTime[wday][0], wakeTime[wday][1], wakeTime[wday][2]);
        timeTo[SUNRISE_END] = msToSunEvent(true);
        timeTo[SUNSET_START] = msToSunEvent(false);
        timeTo[SUNSET_HOLD] = msToSunEvent(false, false);
        timeTo[SLEEP_TIME] = msToLocTime(sleepTime[wday][0], sleepTime[wday][1], sleepTime[wday][2]);
        timeTo[CALCULATE] = msToLocTime(2);

        if (timeTo[SUNRISE_END] < timeTo[WAKE_TIME] + 30 * 60 * 1000)
            timeTo[SUNRISE_END] = timeTo[WAKE_TIME] + 30 * 60 * 1000;

        timeTo[SUNRISE_START] = timeTo[WAKE_TIME] - 30 * 60 * 1000;

        if (timeTo[SUNSET_HOLD] == -1)
            timeTo[SUNSET_HOLD] = timeTo[SUNSET_START] + 30 * 60 * 1000;

        timeTo[SUNSET_END] = timeTo[SLEEP_TIME] - 30 * 60 * 1000;

        if (timeTo[CALCULATE] < 0)
            timeTo[CALCULATE] = msToLocTime(26);

        ESP_LOGI("SNTP", "Calculated Schedule (ms)\n\nSunrise Start:\t%- 10i\nSunrise End:\t%- 10i\nSunset Start:\t%- 10i\nSunset Hold:\t%- 10i\nSunset End:\t%- 10i\nRecalculation:\t%- 10i\n\n", timeTo[SUNRISE_START], timeTo[SUNRISE_END], timeTo[SUNSET_START], timeTo[SUNSET_HOLD], timeTo[SUNSET_END], timeTo[CALCULATE]);

        for (int i = 0; i < 8; i++)
        {
            if (timeTo[i] > 0)
            {
                if (i == SUNRISE_START && timeTo[CALCULATE] < timeTo[SUNRISE_START])
                {
                    eventNow = CALCULATE;
                    return;
                }
                eventNow = i;
                return;
            }
        }
        eventNow = 8;
    }
}