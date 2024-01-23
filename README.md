# An Indoor Sun

This is an ESP32 lighting project that delays the rising and setting time of the outdoor sun, so that the user does not need to find himself unexpectedly sitting in the dark when the sun sets.
This indoor sun rises twice a day - once in the morning to wake up the user, setting after a set period of time, and again in the eveing, setting at bed time. 
The device polls for time using NTP, and is able to accurately calculate sunrise and sunset times given the longitude and latitude of the user.

## Installation

1. Create a new ESP-IDF sample project in Visual Studios
2. Replace all files from `main` folder in sample project with files from `main` in this project
3. Replace `CMakeLists.txt`
4. Add `components` folder
5. Open *settings* (bottom row of window) and configure accordingly. (Tip: To find your Longitude and Latitude, open [Google Maps](https://www.google.com/maps), search for your location and right-click on the exact point to display the Latitude and Longitude.) 

## References

*WiFi and SNTP:*

- [roughleaf](https://embeddedtutorials.com/)

*LED Strip:*

- [ESP-IDF Examples](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/led_strip)

*OLED Display:*

- [Espressif Components](https://components.espressif.com/components/espressif/ssd1306)

- [nopnop2002](https://github.com/nopnop2002/esp-idf-ssd1306/tree/master)

- [Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
