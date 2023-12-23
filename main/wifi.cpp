#include "wifi.h"

/*
todo
add function to reset config after using set functions
*/

namespace WIFI {
    std::mutex WiFi::_mutx{};
    WiFi::state_e WiFi::_state{state_e::NOT_INITIALISED};
    wifi_init_config_t WiFi::_wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t WiFi::_wifi_cfg = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold = {
                .authmode = WIFI_SCAN_AUTH_MODE_THRESHOLD,
            },
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
            .sae_pwe_h2e = WIFI_SAE_MODE,
            .sae_h2e_identifier = H2E_IDENTIFIER,
        },
    };

    u_int8_t WiFi::_max_retry = CONFIG_ESP_MAXIMUM_RETRY;
    u_int8_t WiFi::_retry_num = 0;

    WiFi::WiFi() {};

    void WiFi::wifi_event_handler(  void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
        if(WIFI_EVENT == event_base) {
            const wifi_event_t event_type {static_cast<wifi_event_t>(event_id)};

            switch(event_type) {
                case WIFI_EVENT_STA_START: {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::READY_TO_CONNECT;

                    if (ESP_OK == esp_wifi_connect()) {
                        _state = state_e::CONNECTING;
                    }
                    break;
                }

                case WIFI_EVENT_STA_CONNECTED: {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::WAITING_FOR_IP;
                    break;
                }

                case WIFI_EVENT_STA_DISCONNECTED: {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::DISCONNECTED;

                    if(_retry_num < _max_retry) {
                        _retry_num ++;
                        ESP_LOGI("Wifi Station", "Retrying Connection to WiFi");
                        if(ESP_OK == esp_wifi_connect()) {
                            _state = state_e::CONNECTING;
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    void WiFi::ip_event_handler(    void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
        if(IP_EVENT == event_base) {
            const ip_event_t event_type {static_cast<ip_event_t>(event_id)};

            switch(event_type) {
                case IP_EVENT_STA_GOT_IP: {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::CONNECTED;
                    _retry_num = 0;
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                    ESP_LOGI("Wifi Station", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
                    break;
                }

                case IP_EVENT_STA_LOST_IP: {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    if(state_e::DISCONNECTED != _state) {
                        _state = state_e::WAITING_FOR_IP;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    esp_err_t WiFi::begin() {
        std::lock_guard<std::mutex> connect_guard(_mutx);

        esp_err_t status {ESP_OK};

        switch(_state) {
            case state_e::READY_TO_CONNECT:
            case state_e::DISCONNECTED:
            case state_e::CONNECTING:
            case state_e::WAITING_FOR_IP:
            case state_e::CONNECTED:
                break;
            case state_e::NOT_INITIALISED:
            case state_e::INITIALISED:
            case state_e::ERROR:
                status = ESP_FAIL;
                break;
        }
        return status;
    }

    esp_err_t WiFi::_init() {
        std::lock_guard<std::mutex> mutx_guard(_mutx);

        esp_err_t status{ESP_OK};

        if(state_e::NOT_INITIALISED == _state) {
            status |= esp_netif_init();
            if(ESP_OK == status) {
                const esp_netif_t *const p_netif = esp_netif_create_default_wifi_sta();

                if(!p_netif) {
                    status = ESP_FAIL;
                }
            }

            if(ESP_OK == status) {
                status = esp_wifi_init(&_wifi_init_cfg);
            }

            if(ESP_OK == status) {
                status = esp_event_handler_instance_register(WIFI_EVENT,
                                                                ESP_EVENT_ANY_ID,
                                                                &wifi_event_handler,
                                                                nullptr,
                                                                nullptr);
            }

            if(ESP_OK == status) {
                status = esp_event_handler_instance_register(IP_EVENT,
                                                                ESP_EVENT_ANY_ID,
                                                                &ip_event_handler,
                                                                nullptr,
                                                                nullptr);
            }

            if(ESP_OK == status) {
                status = esp_wifi_set_mode(WIFI_MODE_STA);
            }

            if(ESP_OK == status) {
                status = esp_wifi_set_config(WIFI_IF_STA, &_wifi_cfg);
            }

            if(ESP_OK == status) {
                status = esp_wifi_start();
            }

            if(ESP_OK == status) {
                _state = state_e::INITIALISED;
            }
        }

        else if(state_e::ERROR == _state) {
            _state = state_e::NOT_INITIALISED;
        }

        return status;
    }

    // void WiFi::Set_Credentials(const char *ssid, const char *password) {
    //     _wifi_cfg = {
    //         .sta = {
    //             .ssid = CONFIG_ESP_WIFI_SSID,
    //             .password = CONFIG_ESP_WIFI_PASSWORD,
    //         },
    //     };
    //     _retry_num = 0;

    // }

    // void WiFi::Set_Others(const wifi_auth_mode_t *scan_auth_mode_threshold, const wifi_sae_pwe_method_t *sae_mode, const char *h2e_identifier) {
    //     _wifi_cfg.sta.threshold.authmode = *scan_auth_mode_threshold;
    //     _wifi_cfg.sta.sae_pwe_h2e = *sae_mode;
    //     memcpy(_wifi_cfg.sta.sae_h2e_identifier, h2e_identifier, std::min(strlen(h2e_identifier), sizeof(_wifi_cfg.sta.sae_h2e_identifier)));
    // }

    // void WiFi::Set_Max_Retry(const u_int8_t *max_retry) {
    //     _max_retry = *max_retry;
    //     _retry_num = 0;
    // }

    esp_err_t WiFi::init() {
        return _init();
    }
}