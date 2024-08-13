#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../pixels.h"
#include "ws2812_strip_encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"

static uint8_t * rgbData;
static uint16_t rgbDataCounter;
static uint8_t * rgbDataPointer;
static uint16_t lookupTable[4];

static const char *TAG = "LED_STRIP";
static rmt_encoder_handle_t led_encoder;
static rmt_channel_handle_t led_chan = NULL;

#if CONFIG_WS2812_LED_TYPE_RGB
#define BITS_PER_LED_CMD	24
#elif CONFIG_WS2812_LED_TYPE_RGBW
#define BITS_PER_LED_CMD	32
#endif

#define LCD_HOST     "LED_STRIP"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)

// gamma correction at driver level....
const uint8_t gamma[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6,
		6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12,
		13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21,
		21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32,
		33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
		49, 50, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67,
		68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89, 90,
		92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
		115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138,
		140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167,
		169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198,
		200, 203, 205, 208, 210, 213, 215, 218, 220, 223, 225, 228, 231, 233,
		236, 239, 241, 244, 247, 249, 252, 255 };

esp_err_t _rmt_init() {
  
  ESP_LOGI(TAG, "Create RMT TX channel");
  rmt_tx_channel_config_t tx_chan_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
      .gpio_num = CONFIG_CABINET_LIGHTING_LED_SIGNAL,
      .mem_block_symbols = 64, // increase the block size can make the LED less flickering
      .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
      .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));


  led_strip_encoder_config_t encoder_config = {
      .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
  };

  ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));
  ESP_ERROR_CHECK(rmt_enable(led_chan));

  return ESP_OK;
}
/*
 * Prepare data and lookup table
 */
void _data_init(uint16_t ledCount) {
	rgbData = malloc(ledCount * 3);
	uint16_t b = ledCount;
	uint8_t* ledPointer;
	while (b > 0) {
		b--;
		ledPointer = rgbData + b;
		*ledPointer = 0;
		*(ledPointer + 1) = 0;
		*(ledPointer + 2) = 0;
	}
}

esp_err_t led_strip_init(uint16_t ledCount) {
  ESP_ERROR_CHECK(_rmt_init());
	_data_init(ledCount);

  return ESP_OK;
}

/*
 * When RGB data is ready, call this function to send it to WS2812x LEDs
 */
esp_err_t led_strip_sendData(Pixels* pixelData) {
  // rgb ->  g, r, b
  uint16_t i = pixelData->pixelCount;
  rmt_transmit_config_t tx_config = {
      .loop_count = 0, // no transfer loop
  };

  if (i == 0) {
    return ESP_OK;    //no need to send anything
  }
  uint8_t* pixRef = pixelData->pixelData;
  uint8_t* dataRef = rgbData;

  while (i > 0) {
    i--;
    *(dataRef) = gamma[*(pixRef + 1)];
    *(dataRef + 1) = gamma[*(pixRef)];
    *(dataRef + 2) = gamma[*(pixRef + 2)];

    dataRef += 3;
    pixRef += 3;
  }
	// for (int j = i; j < EXAMPLE_LED_NUMBERS; j += 3) {
	// 	// Build RGB pixels
	// 	hue = j * 360 / EXAMPLE_LED_NUMBERS + start_rgb;
	// 	led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
	// 	led_strip_pixels[j * 3 + 0] = green;
	// 	led_strip_pixels[j * 3 + 1] = blue;
	// 	led_strip_pixels[j * 3 + 2] = red;
	// }

	// Flush RGB values to LEDs
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, rgbData, pixelData->pixelCount * 3, &tx_config));
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

  return ESP_OK;
}
