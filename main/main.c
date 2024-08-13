#include <stdio.h>
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "./mqtt/mqtt.h"
#include "./server.h"
#include "./filesystem.h"
#include "./wifi.h"
#include "./settings.h"
#include "./appstate.h"
#include "./driver/led_strip.h"
#include "./driver/sound.h"
#include "./animation.h"
#include "./animation_loop.h"
#include "./motion.h"
#include "./mqtt/homeassistant.h"
#include "./util/event.h"

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  sound_init();
  appstate_init();
  init_fs();
  init_settings();
  ESP_ERROR_CHECK(led_strip_init(LED_COUNT));
  motion_init();
  animation_init();
  animation_loop_init();

  wifi_start();
  ESP_ERROR_CHECK(mqtt_init());
  start_rest_server(CONFIG_CABINET_LIGHTING_WEB_MOUNT_POINT);
}