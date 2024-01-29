set(SOURCES main.cpp
            wifi.cpp
            sntp.cpp
            led_strip.cpp
            led_blink.cpp
            ssd1306_fonts.cpp
            ssd1306.cpp)
            
idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS . ../components
                        REQUIRES esp_wifi nvs_flash esp_event esp_netif lwip driver)