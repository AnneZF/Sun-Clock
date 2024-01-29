# An Indoor Sun

This is an ESP32 lighting project that delays the rising and setting time of the outdoor sun so that the user does not need to find himself unexpectedly sitting in the dark when the sun sets.
This indoor sun rises twice daily - once in the morning to wake the user, setting after a set period, and again in the evening, setting at bedtime. 
The device polls for time using NTP, and can accurately calculate sunrise and sunset times given the longitude and latitude of the user.

## Installation

1. Create a new ESP-IDF sample project
2. Copy files into the project folder
3. Open *settings* (bottom row of window) and configure accordingly.

Tips:
- To find your Longitude and Latitude, open [Google Maps](https://www.google.com/maps), search for your location and right-click on the exact point to display the Latitude and Longitude.
- Change `RTC Clock Source` to `Internal 8.5MHz oscillator, divided by 256 (~33kHz)` for more accurate oscillator cycles

## References

*LED Strip (WS2812):*

- [ESP-IDF Examples](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/led_strip)

*OLED Display (SSD1306):*

- [Espressif Components](https://components.espressif.com/components/espressif/ssd1306)

- [nopnop2002](https://github.com/nopnop2002/esp-idf-ssd1306/tree/master)

- [Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

*Sunrise/Sunset Algorithm:*

- [Ed Williams' Aviation](https://edwilliams.org/sunrise_sunset_algorithm.htm)

*WiFi and SNTP:*

- [roughleaf](https://embeddedtutorials.com/)
