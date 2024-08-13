#include "cJSON.h"
#include "esp_log.h"
#include "string.h"
#include "./mqtt.h"
#include "./homeassistant.h"
#include "../util/event.h"
#include "../util/rgbfunc.h"
#include "../motion.h"
#include "../appstate.h"
#include "../animation.h"

#define TAG "HOMEASSISTANT"

extern app_state_t appstate;
extern const animation_t* animations[];

void homeassistant_motion_publish_state() {
  int result =  mqtt_publish("homeassistant/binary_sensor/kitchen/state", appstate.isOccupied ? "ON" : "OFF");

  if (result < 0) {
      ESP_LOGE(TAG, "Failed to publish motion state, %d", result);
      return;
  }
  ESP_LOGI(TAG, "Published Motion Detected");
}

void homeassistant_light_publish_state() {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "brightness", appstate.settings->occupied.brightness);
  cJSON_AddStringToObject(root, "color_mode", "rgb");
  cJSON *color = cJSON_AddObjectToObject(root, "color");
  cJSON_AddNumberToObject(color, "r", red(appstate.settings->occupied.color));
  cJSON_AddNumberToObject(color, "g", green(appstate.settings->occupied.color));
  cJSON_AddNumberToObject(color, "b", blue(appstate.settings->occupied.color));
  cJSON_AddStringToObject(root, "effect", animations[appstate.settings->occupied.animation]->name);
  cJSON_AddStringToObject(root, "state", appstate.isOccupied ? "ON" : "OFF");

  
  char *state = cJSON_Print(root);
  
  mqtt_publish("homeassistant/light/kitchen/state", state);
  
  cJSON_Delete(root);
}

void motion_state_callback(const char* sender, const void * eventArgs) {
  homeassistant_motion_publish_state();
  homeassistant_light_publish_state();
}

void settings_change_callback(const char* sender, const void *eventArgs) {
  homeassistant_light_publish_state();
}

void homeassistant_light_set(const char *topic, const char *data, void *arg) {
  ESP_LOGI(TAG, "Received set command for light");

  /// DO IT

  cJSON *root = cJSON_Parse(data);
  if (root == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      ESP_LOGE(TAG, "Error parsing update settings before: %s\n", error_ptr);
    }
    return;
  }
  
  bool isChanged = false;
  lighting_settings *update_settings = (lighting_settings*)malloc(sizeof(lighting_settings));
  memcpy(update_settings, appstate.settings, sizeof(lighting_settings));

  if (cJSON_HasObjectItem(root, "brightness")) {
    int brightness = cJSON_GetObjectItem(root, "brightness")->valueint;
    if (brightness != appstate.settings->occupied.brightness) {
      update_settings->occupied.brightness = brightness;
      isChanged = true;
    }
  }
  if (cJSON_HasObjectItem(root, "effect")) {
    char* effect = cJSON_GetObjectItem(root, "effect")->valuestring;
    if (strcmp(effect, animations[appstate.settings->occupied.animation]->name) != 0) {
      for (int i = 0; i < ANIMATION_COUNT; i++) {
        if (strcmp(effect, animations[i]->name) == 0) {
          update_settings->occupied.animation = i;
          isChanged = true;
          break;
        }
      }
    }
  }
  if (cJSON_HasObjectItem(root, "color")) {
    cJSON *color = cJSON_GetObjectItem(root, "color");
    if (cJSON_HasObjectItem(color, "r") && cJSON_HasObjectItem(color, "g") && cJSON_HasObjectItem(color, "b")) {
      int r = cJSON_GetObjectItem(color, "r")->valueint;
      int g = cJSON_GetObjectItem(color, "g")->valueint;
      int b = cJSON_GetObjectItem(color, "b")->valueint;
      int color = combine(r, g, b);
      if (color != appstate.settings->occupied.color) {
        update_settings->occupied.color = color;
        isChanged = true;
      }
    }
  }

  cJSON_Delete(root);

  if (isChanged) {
    settings_update(TAG, update_settings);
  }
  free(update_settings);
}

void homeassistant_subscribe_light() {
  mqtt_subscribe_callback("homeassistant/light/kitchen/set", homeassistant_light_set, NULL);
}

void homeassistant_motion_available() {
  int result = mqtt_publish("homeassistant/binary_sensor/kitchen/availability", "online");
  if (result < 0) {
    ESP_LOGE(TAG, "Failed to publish kitchen motion sensor availability, %d", result);
  } else {
    ESP_LOGI(TAG, "Published Motion Available");
  }
}

void homeassistant_light_available() {
  esp_err_t result = mqtt_publish("homeassistant/light/kitchen/availability", "online");
  if (result < 0) {
    ESP_LOGE(TAG, "Failed to publish kitchen light availability, %d", result);
  } else {
    ESP_LOGI(TAG, "Published Light Available");
  }
}

void homeassistant_publish_motion_config() {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "device_class", "motion");
  cJSON_AddStringToObject(root, "unique_id", "kitchen-motion-7134");
  cJSON_AddStringToObject(root, "state_topic", "homeassistant/binary_sensor/kitchen/state");

  cJSON *availabililty = cJSON_AddObjectToObject(root, "availability");
  cJSON_AddStringToObject(availabililty, "topic", "homeassistant/binary_sensor/kitchen/availability");
  cJSON_AddStringToObject(availabililty, "payload_available", "online");
  cJSON_AddStringToObject(availabililty, "payload_not_available", "offline");
  
  cJSON *device = cJSON_AddObjectToObject(root, "device");
  cJSON_AddStringToObject(device, "name", "Kitchen Undercabinet Lighting");
  cJSON *identifiers = cJSON_AddArrayToObject(device, "identifiers");
  cJSON_AddItemToArray(identifiers, cJSON_CreateString("undercabinet-lighting-7134"));


  cJSON_AddNumberToObject(root, "qos", 1);
  cJSON_AddBoolToObject(root, "retain", false);

  cJSON_AddStringToObject(root, "payload_on", "ON");
  cJSON_AddStringToObject(root, "payload_off", "OFF");

  char *config = cJSON_Print(root);
  
  mqtt_publish("homeassistant/binary_sensor/kitchen/config", config);
  
  cJSON_Delete(root);
}

void homeassistant_publish_light_config() {

  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "name", "Kitchen Undercabinet Lighting");
  cJSON_AddStringToObject(root, "unique_id", "kitchen-undercabinet-light-7134");
  cJSON_AddStringToObject(root, "schema", "json");
  cJSON_AddStringToObject(root, "command_topic", "homeassistant/light/kitchen/set");
  cJSON_AddStringToObject(root, "state_topic", "homeassistant/light/kitchen/state");
  cJSON_AddStringToObject(root, "availability_topic", "homeassistant/light/kitchen/availability");

  cJSON *device = cJSON_AddObjectToObject(root, "device");  
  cJSON *identifiers = cJSON_AddArrayToObject(device, "identifiers");
  cJSON_AddItemToArray(identifiers, cJSON_CreateString("undercabinet-lighting-7134"));

  cJSON_AddBoolToObject(root, "brightness", true);
  cJSON_AddNumberToObject(root, "brightness_scale", 15);
  cJSON *color_modes = cJSON_AddArrayToObject(root, "supported_color_modes");
  cJSON_AddItemToArray(color_modes, cJSON_CreateString("rgb"));

  cJSON_AddBoolToObject(root, "effect", true);
  cJSON *effects = cJSON_AddArrayToObject(root, "effect_list");
  for (int i = 0; i < ANIMATION_COUNT; i++) {
    cJSON_AddItemToArray(effects, cJSON_CreateString(animations[i]->name));
  }

  cJSON_AddNumberToObject(root, "qos", 1);
  cJSON_AddBoolToObject(root, "retain", false);

  char *config = cJSON_Print(root);
  
  mqtt_publish("homeassistant/light/kitchen/config", config);
  
  cJSON_Delete(root);
}

void homeassistant_publish_discovery() {
  homeassistant_publish_light_config();
  homeassistant_publish_motion_config();
  homeassistant_motion_available();
  homeassistant_light_available();

  homeassistant_light_publish_state();
  homeassistant_motion_publish_state();
}

void homeassistant_on_status(const char *topic, const char *data, void *arg) {
  if (!strcmp(data, "online")) {
    ESP_LOGI(TAG, "Home Assistant is back online.");     
    homeassistant_publish_discovery();
  } else {
    ESP_LOGI(TAG, "Home Assistant is offline.");     
  }
}
void homeassistant_subscribe_birth() {
  mqtt_subscribe_callback("homeassistant/status", homeassistant_on_status, NULL);
  homeassistant_light_publish_state();
  homeassistant_motion_publish_state();
}

void homeassistant_init() {
  homeassistant_subscribe_light();
  homeassistant_subscribe_birth();
  homeassistant_publish_discovery();

  homeassistant_motion_publish_state();
  homeassistant_light_publish_state();

  event_add_handler(EVENT_MOTION_STATE_CHANGED, motion_state_callback);
  event_add_handler(EVENT_LIGHT_SETTINGS_UPDATED, settings_change_callback);
}