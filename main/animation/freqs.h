/*
 * freqs.h
 *
 *  Created on: Feb 3, 2016
 *      Author: efess
 */
#include <stdint.h>
#include "../animation.h"
#include "../pixels.h"
#include "../settings.h"

#define BIN_COUNT 16

typedef struct {
	uint8_t freqs[BIN_COUNT];
	uint8_t brightness;
	uint32_t color;
} FreqsState;

animation_t* freqs_get_animation();