/*
 * motion.c
 *
 *  Created on: Jan 9, 2016
 *      Author: efess
 */
#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "motion.h"
#include "appstate.h"
#include "./util/event.h"

#define MOTION_PIN CONFIG_CABINET_LIGHTING_GPIO_MOTION_SENSOR
#define GPIO_INPUT_PIN_SEL  (1ULL<<MOTION_PIN)
#define ESP_INTR_FLAG_DEFAULT 0

#define TAG "motion"

extern app_state_t appstate;

uint32_t volatile _secondsCounter = 3600;
uint32_t volatile _lastOccupiedDetected = 0;

static QueueHandle_t gpio_evt_queue = NULL;
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
  uint32_t gpio_num = (uint32_t) arg;
  
  // xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);

  uint8_t state = gpio_get_level(gpio_num);
  if (gpio_num == MOTION_PIN && state > 0) {
    _lastOccupiedDetected = _secondsCounter;
  }
}

void _motion_change_state(bool occupied) {
  appstate.isOccupied = occupied;
  event_trigger(TAG, EVENT_MOTION_STATE_CHANGED, (void*)appstate.isOccupied);
}

static void gpio_queue_worker(void* arg)
{
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      uint8_t state = gpio_get_level(io_num);
      if (io_num == MOTION_PIN) {
        if (state > 0) {
          ESP_LOGI(TAG, "Motion Detected");
          _lastOccupiedDetected = _secondsCounter;
        }
      } else {
          ESP_LOGI(TAG, "Unknown GPIO state change on pin %lu ", io_num);
      }
    }
  }
}

void _motion_init_gpio(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    //create a queue to handle gpio event from isr
    // gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    // xTaskCreate(gpio_queue_worker, "gpio_queue_worker", 1024, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(MOTION_PIN, gpio_isr_handler, (void*) MOTION_PIN);
}

static void timer_callback(void* arg)
{ 
  _secondsCounter++;
  
  bool newOccupied = _lastOccupiedDetected + appstate.settings->occupiedTimeout > _secondsCounter;
  if (appstate.isOccupied != newOccupied) {
    _motion_change_state(newOccupied);
  }
}

void _motion_init_timer()
{
  const esp_timer_create_args_t periodic_timer_args = {
          .callback = &timer_callback,
          /* name is optional, but may help identify the timer when debugging */
          .name = "periodic"
  };

  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));
}

void motion_init(void)
{
	_motion_init_timer();
	_motion_init_gpio();
}
