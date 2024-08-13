/*
 * settings.c
 *
 *  Created on: Jan 9, 2016
 *      Author: efess
 */
#include "esp_log.h"
#include <string.h>
#include "./settings.h"
#include "./filesystem.h"
#include "./appstate.h"
#include "./util/event.h"

extern app_state_t appstate;
static const char *TAG = "settings";

static const uint32_t testPallete[16] = {
	0x5500AB, 0x84007C, 0xB5004B, 0xE5001B,
	0xE81700, 0xB84700, 0xAB7700, 0xABAB00,
	0xAB5500, 0xDD2200, 0xF2000E, 0xC2003E,
	0x8F0071, 0x5F00A1, 0x2F00D0, 0x0007F9
};

uint8_t settings_getBrightness(uint8_t settings_brightness)
{
	return settings_brightness << 4;
}

void settings_set_default() {
  appstate.settings->occupied.color = 0x0000FF;
  appstate.settings->occupied.animation = 7;
  appstate.settings->occupied.brightness = 5;
  appstate.settings->occupied.transition = 1;
  memcpy(appstate.settings->occupied.colorPallete, testPallete, sizeof(testPallete));

  appstate.settings->unoccupied.color = 0x00FF00;
  appstate.settings->unoccupied.animation = 2;
  appstate.settings->unoccupied.brightness = 2;
  appstate.settings->unoccupied.transition = 1;
  memcpy(appstate.settings->unoccupied.colorPallete, testPallete, sizeof(testPallete));

  appstate.settings->occupiedTimeout = 300; // timeout seconds default (5 Mins)
  appstate.settings->alwaysOn = 0;
}

void init_settings() 
{
  char *storedSettings;
  size_t storedSize = 0;
  if (ESP_FAIL != fs_read_file("/settings", &storedSettings, &storedSize)) {
    if (sizeof(lighting_settings) == storedSize) {
      memcpy(appstate.settings, storedSettings, sizeof(lighting_settings));
      
      ESP_LOGI(TAG, "Loaded settings from store");
      return;
    } else {
      ESP_LOGE(TAG, "Size of settings changed. Resetting to default");
    }
  }
  settings_set_default();
  if (ESP_FAIL == fs_write_file("/settings", (char*)appstate.settings, sizeof(lighting_settings))) {
    ESP_LOGE(TAG, "Failed to write default settings");
  } else {
    ESP_LOGI(TAG, "Default settings written");
  }
  return;
}

void settings_update(const char* sender, lighting_settings* newSettings) {
  memcpy(appstate.settings, newSettings, sizeof(lighting_settings));
  event_trigger(sender, EVENT_LIGHT_SETTINGS_UPDATED, NULL);

  if (ESP_FAIL == fs_write_file("/settings", (char*)appstate.settings, sizeof(lighting_settings))) {
    ESP_LOGE(TAG, "Failed to write new settings");
  } else {
    ESP_LOGI(TAG, "Updated settings written");
  }
}