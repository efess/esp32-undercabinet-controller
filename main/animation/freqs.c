/*
 * freqs.c
 *
 *  Created on: Feb 3, 2016
 *      Author: efess
 */

#include "string.h"
#include "math.h"
#include "esp_dsp.h"
#include "esp_log.h"
#include "math.h"
#include "./driver/sound.h"
#include "../util/rgbfunc.h"
#include "freqs.h"

#define TAG "freqs_animation"

#define N_SAMPLES 1024

int N = N_SAMPLES;

// Window coefficients
__attribute__((aligned(16)))
float wind[N_SAMPLES];
// working complex array
__attribute__((aligned(16)))
float y_cf[N_SAMPLES];
// Pointers to result arrays
float *y1_cf = &y_cf[0];
float *y2_cf = &y_cf[N_SAMPLES];

// Sum of y1 and y2
__attribute__((aligned(16)))
float sum_y[N_SAMPLES / 2];

void freqs_setup(void *g_state, room_state_settings* settings)
{
	FreqsState *state = g_state;

	memset(state, 0, sizeof(FreqsState));
	state->brightness = settings_getBrightness(settings->brightness);
	state->color = settings->color;
}

void freqs_frame(void *g_state, Pixels* pixels)
{
	FreqsState *state = g_state;

    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret  != ESP_OK) {
        ESP_LOGE(TAG, "Not possible to initialize FFT. Error = %i", ret);
        return;
    }
	audio_sample_t *audioBuffer = malloc(sizeof(audio_sample_t) * N_SAMPLES);
	size_t read_samples = sound_read_samples(audioBuffer, N_SAMPLES);
	if (read_samples == 0) {
		ESP_LOGI(TAG, "No sound samples read");
		free(audioBuffer);
		return;
	}

	float sampleWindow = INT16_MAX;
	
    // Generate hann window
    dsps_wind_hann_f32(wind, N);

    // Convert two input vectors to one complex vector
    for (int i = 0 ; i < N_SAMPLES ; i++) {
		float sampleValue = i < read_samples ? (fmin((float)(int32_t)audioBuffer[i], sampleWindow) / sampleWindow) : 0;
        y_cf[i * 2 + 0] = sampleValue * wind[i];
        y_cf[i * 2 + 1] = sampleValue;
    }

	free(audioBuffer);

    // FFT
    unsigned int start_b = dsp_get_cpu_cycle_count();
    dsps_fft2r_fc32(y_cf, N);
    unsigned int end_b = dsp_get_cpu_cycle_count();
    // Bit reverse
    dsps_bit_rev_fc32(y_cf, N);
    // Convert one complex vector to two complex vectors
    dsps_cplx2reC_fc32(y_cf, N);

    for (int i = 0 ; i < N / 2 ; i++) {
        y1_cf[i] = 10 * log10f((y1_cf[i * 2 + 0] * y1_cf[i * 2 + 0] + y1_cf[i * 2 + 1] * y1_cf[i * 2 + 1]) / N);
        y2_cf[i] = 10 * log10f((y2_cf[i * 2 + 0] * y2_cf[i * 2 + 0] + y2_cf[i * 2 + 1] * y2_cf[i * 2 + 1]) / N);
        // Simple way to show two power spectrums as one plot
        sum_y[i] = fmax(y1_cf[i], y2_cf[i]);
    }

    // Show power spectrum in 64x10 window from -100 to 0 dB from 0..N/4 samples
    // ESP_LOGW(TAG, "Signal");
    // dsps_view(y1_cf, N / 2, 64, 10,  -60, 40, '|');
    // ESP_LOGW(TAG, "Signal x2");
    // dsps_view(y2_cf, N / 2, 64, 10,  -60, 40, '|');
    // ESP_LOGW(TAG, "Signals x1 and x2 on one plot");
    // dsps_view(sum_y, N / 2, 64, 10,  -60, 40, '|');
    // ESP_LOGI(TAG, "FFT for %i complex points take %i cycles", N, end_b - start_b);

	// float max = 0;
	// float min = 0;
	// float avg = 0;

	// for(int i = 0; i < N/2; i++) {
	// 	float cur =  y1_cf[i];
	// 	if (cur > max) {
	// 		max = cur;
	// 	}
	// 	if (cur < min) {
	// 		min = cur;
	// 	}
	// 	avg += cur;
	// }
	// avg = avg / read_samples;
	// ESP_LOGI(TAG, "FREQ (READ %d): Max: %.2f, Min: %.2f, Avg: %.2f",  N/2,  max, min, avg);

	float lowerBound = -60.0f;
	float upperBound = 40.0f;

	float dataRange = upperBound - lowerBound;
	float* data = y1_cf;
	uint16_t dataLen = N/2;

	uint8_t normalBrightness = 0x66;
	float brightnessRange = 0xff - normalBrightness;
	
	uint8_t brightnesses[LED_COUNT] = {0};

	for(int i = 0; i < dataLen; i++)
	{
		uint16_t pixelNum = ((float)i / (float)dataLen) * LED_COUNT;
		float clamp = fminf(fmaxf(data[i], lowerBound), upperBound);
		float power = (100.0f + (clamp - upperBound)) / 100.0f; // Clamp and normalize to %
		float brightness = normalBrightness + (brightnessRange * power);
		if (brightness > brightnesses[pixelNum]) {
			brightnesses[pixelNum] = brightness;
		}
	}

	for(int i = 0; i < LED_COUNT; i++)
	{
		pixels_setPixelColor(pixels, i, changeBrightness(state->color, brightnesses[i]));//state->brightness));
    }
}

animation_t freqsAnimation = {
  .id = 7,
  .frameDelay = 20,
  .setupFn = freqs_setup,
  .frameFn = freqs_frame,
  .name = "Sound Beats"
};

animation_t* freqs_get_animation() {
	return &freqsAnimation;
}