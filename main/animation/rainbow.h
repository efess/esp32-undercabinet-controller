/*
 * rainbow.h
 *
 *  Created on: Jan 1, 2016
 *      Author: efess
 */

#include <stdint.h>
#include "../animation.h"
#include "../pixels.h"
#include "../settings.h"

typedef struct {
	uint8_t brightness;
	uint8_t step;
} RainbowState;

animation_t* rainbow_get_animation();