#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "./util/event.h"
#include "./appState.h"
#include "./driver/led_strip.h"
#include "./animation.h"

#define TAG "animation_loop"

static QueueHandle_t gpio_evt_queue = NULL;

bool _stateChange = true;

extern app_state_t appstate;

void _state_change_callback(const char* sender, const void *data) {
  _stateChange = true;
}

void _task_loop()
{ 
  while(1) {
    if(_stateChange)
    {
      ESP_LOGI(TAG, "Running Animation Change State - current occupancy is %d", appstate.isOccupied);
      _stateChange = false;
      animation_changeState(&appstate);
    }

    animation_runFrame(appstate.pixels);

    led_strip_sendData(appstate.pixels);
    
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void _animation_loop_init_timer()
{
  xTaskCreatePinnedToCore(
    _task_loop,    // Function that should be called
    "animation-loop-task",   // Name of the task (for debugging)
    6000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL,             // Task handle
    1);                // core to run task);
}

void animation_loop_init()
{
  _animation_loop_init_timer();

  event_add_handler(EVENT_LIGHT_SETTINGS_UPDATED, _state_change_callback);
  event_add_handler(EVENT_MOTION_STATE_CHANGED, _state_change_callback);
}