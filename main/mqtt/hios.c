#include "esp_log.h"
#include "string.h"
#include "cJSON.h"
#include "./mqtt.h"
#include "./hios.h"
#include "../settings.h"
#include "../appstate.h"
#include "../util/cdecode.h"

extern app_state_t appstate;

#define HIOS_TOPIC_SUB_OPTIONS_UPDATE "/home/kitchen/cabinet/lights/update"
#define HIOS_TOPIC_SUB_OPTIONS_REQUEST "/home/kitchen/cabinet/lights/request"
#define HIOS_TOPIC_PUB_OPTIONS_RESPONSE "/home/kitchen/cabinet/lights/response"

#define TAG "HIOS"

void hios_update_settings(const char *topic, const char *data, void *arg) {
    ESP_LOGI(TAG, "Received update settings event");
    lighting_settings *update_settings = (lighting_settings*)malloc(sizeof(lighting_settings));

    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "Error parsing update settings before: %s\n", error_ptr);
        }
        return;
    }

    update_settings->motion_control = cJSON_GetObjectItem(root, "motionControl")->valueint;
    update_settings->occupiedTimeout = cJSON_GetObjectItem(root, "occupiedTimeout")->valueint;
    update_settings->alwaysOn = cJSON_GetObjectItem(root, "alwaysOn")->valueint;

    cJSON *occupied = cJSON_GetObjectItem(root, "occupied");
    update_settings->occupied.brightness = cJSON_GetObjectItem(occupied, "brightness")->valueint;
    update_settings->occupied.transition = cJSON_GetObjectItem(occupied, "transition")->valueint;
    update_settings->occupied.animation = cJSON_GetObjectItem(occupied, "animation")->valueint;
    update_settings->occupied.color = cJSON_GetObjectItem(occupied, "color")->valueint;
    cJSON *occupiedPallete = cJSON_GetObjectItem(occupied, "pallete");
    for(int i = 0; i < 16; i++) {
        update_settings->occupied.colorPallete[i] = cJSON_GetArrayItem(occupiedPallete, i)->valueint;
    }

    cJSON *unoccupied = cJSON_GetObjectItem(root, "unoccupied");
    update_settings->unoccupied.brightness = cJSON_GetObjectItem(unoccupied, "brightness")->valueint;
    update_settings->unoccupied.transition = cJSON_GetObjectItem(unoccupied, "transition")->valueint;
    update_settings->unoccupied.animation = cJSON_GetObjectItem(unoccupied, "animation")->valueint;
    update_settings->unoccupied.color = cJSON_GetObjectItem(unoccupied, "color")->valueint;
    cJSON *unoccupiedPallete = cJSON_GetObjectItem(unoccupied, "pallete");
    for(int i = 0; i < 16; i++) {
        update_settings->unoccupied.colorPallete[i] = cJSON_GetArrayItem(unoccupiedPallete, i)->valueint;
    }
    
    if(memcmp(update_settings, appstate.settings, sizeof(lighting_settings)) == 0)
    {
        ESP_LOGI(TAG,"No change in update\r\n");
    } else {
        settings_update(TAG, update_settings);
    }
    free(update_settings);
}

void hios_publish_settings(const char *topic, const char *data, void *arg) {
    ESP_LOGI(TAG, "Received request settings event");
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "motionControl", appstate.settings->motion_control);
    cJSON_AddNumberToObject(root, "occupiedTimeout", appstate.settings->occupiedTimeout);
    cJSON_AddBoolToObject(root, "alwaysOn", appstate.settings->alwaysOn == 1 ? true : false);

    cJSON *occupied = cJSON_AddObjectToObject(root, "occupied");
    cJSON_AddNumberToObject(occupied, "brightness", appstate.settings->occupied.brightness);
    cJSON_AddNumberToObject(occupied, "transition", appstate.settings->occupied.transition);
    cJSON_AddNumberToObject(occupied, "animation", appstate.settings->occupied.animation);
    cJSON_AddNumberToObject(occupied, "color", appstate.settings->occupied.color);
    cJSON *occupiedPallete = cJSON_AddArrayToObject(occupied, "pallete");
    for(int i = 0; i < 16; i++) {
        cJSON_AddItemToArray(occupiedPallete, cJSON_CreateNumber(appstate.settings->occupied.colorPallete[i]));
    }

    cJSON *unoccupied = cJSON_AddObjectToObject(root, "unoccupied");
    cJSON_AddNumberToObject(unoccupied, "brightness", appstate.settings->unoccupied.brightness);
    cJSON_AddNumberToObject(unoccupied, "transition", appstate.settings->unoccupied.transition);
    cJSON_AddNumberToObject(unoccupied, "animation", appstate.settings->unoccupied.animation);
    cJSON_AddNumberToObject(unoccupied, "color", appstate.settings->unoccupied.color);
    cJSON *unoccupiedPallete = cJSON_AddArrayToObject(unoccupied, "pallete");
    for(int i = 0; i < 16; i++) {
        cJSON_AddItemToArray(unoccupiedPallete, cJSON_CreateNumber(appstate.settings->unoccupied.colorPallete[i]));
    }

    char *message = cJSON_Print(root);
    
    mqtt_publish(HIOS_TOPIC_PUB_OPTIONS_RESPONSE, message);
    
    cJSON_Delete(root);
}

void hios_subscribe() {

    mqtt_subscribe_callback(HIOS_TOPIC_SUB_OPTIONS_UPDATE, hios_update_settings, NULL);
    mqtt_subscribe_callback(HIOS_TOPIC_SUB_OPTIONS_REQUEST, hios_publish_settings, NULL);
}

void hios_init() {
    hios_subscribe();
}
