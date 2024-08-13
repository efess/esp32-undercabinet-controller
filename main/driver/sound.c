#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "./sound.h"

#define PIN_CLOCK CONFIG_CABINET_LIGHTING_GPIO_MIC_CLK
#define PIN_SD CONFIG_CABINET_LIGHTING_GPIO_MIC_SD
#define PIN_WS CONFIG_CABINET_LIGHTING_GPIO_MIC_WS // left/right clock

#define AUDIO_CHANNEL_SAMPLE_RATE 16000
#define AUDIO_DMA_BUFFER_COUNT 3

#define IS2_AUDIO_BUFFER_LEN 1023 // max samples for i2s_read

#define TAG "sound"

i2s_chan_handle_t rx_chan;

void sound_init() {
  rx_chan = NULL;
  i2s_chan_config_t i2s_chan_cfg_rx = {
      .id = I2S_NUM_0,
      .role = I2S_ROLE_MASTER,
      .dma_desc_num = AUDIO_DMA_BUFFER_COUNT,
      .dma_frame_num = IS2_AUDIO_BUFFER_LEN,
      .auto_clear = false,
  };
  i2s_std_config_t rx_std_cfg = {
      .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_CHANNEL_SAMPLE_RATE),
      .slot_cfg = {
              .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
              .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
              .slot_mode = I2S_SLOT_MODE_MONO,
              .slot_mask = I2S_STD_SLOT_LEFT,
              .ws_width = 32,
              .ws_pol = false,
              .left_align = true,
              .big_endian = false,
              .bit_order_lsb = false,
              .bit_shift = true,
              
              
      },
      .gpio_cfg = {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = PIN_CLOCK,
              .ws   = PIN_WS,
              .dout = I2S_GPIO_UNUSED,
              .din  = PIN_SD,
              .invert_flags = {
                      .mclk_inv = false,
                      .bclk_inv = true,
                      .ws_inv   = false,
              },
      },
  };

  
  //s_i2s_context = get_i2s_context();
  esp_err_t err = i2s_new_channel(&i2s_chan_cfg_rx, NULL, &rx_chan);
  ESP_ERROR_CHECK(err);
  err = i2s_channel_init_std_mode(rx_chan, &rx_std_cfg);
  ESP_ERROR_CHECK(err);
  i2s_chan_info_t info;
  err = i2s_channel_get_info(rx_chan, &info);
  ESP_ERROR_CHECK(err);
  err = i2s_channel_enable(rx_chan);
  ESP_ERROR_CHECK(err);

  // Set the pulldown resistor on the SD pin
  gpio_set_pull_mode(PIN_SD, GPIO_PULLDOWN_ONLY);
}

size_t sound_read_samples(audio_sample_t *buffer, uint16_t samples_to_read)
{
  // if (buffer_size > IS2_AUDIO_BUFFER_LEN)
  // {
  //   ESP_LOGE(TAG, "Buffer size is too big, must be less than %d", IS2_AUDIO_BUFFER_LEN);
  //   return 0;
  // }

  size_t bytes_per_sample = sizeof(audio_sample_t);
  size_t bytes_read = 0;
  esp_err_t err = i2s_channel_read(rx_chan, buffer, bytes_per_sample * samples_to_read, &bytes_read, 50);
  const size_t count = bytes_read / bytes_per_sample;

  if (err == ESP_OK)
  {
    if (bytes_read > 0)
    {

      return count;
    }
  }
  if (err != ESP_ERR_TIMEOUT)
  {
    ESP_ERROR_CHECK(err);
  } else {
    ESP_LOGI(TAG, "Timeout reading samples, %d", bytes_read);
  }
  return 0;
}