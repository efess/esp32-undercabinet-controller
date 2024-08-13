idf_component_register(SRC_DIRS "." "util" "transition" "driver" "animation" "mqtt"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_netif driver esp_wifi vfs mqtt esp_http_server json littlefs nvs_flash dns_server)