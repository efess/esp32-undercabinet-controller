/*
 * discrete.h
 *
 *  Created on: Jan 11, 2016
 *      Author: efess
 */

#include <stdint.h>
#include "../animation.h"
#include "../pixels.h"
#include "../settings.h"

typedef struct {
	uint8_t brightness;
	uint32_t color;
	uint8_t rendered;
} DiscreteState;

animation_t* discrete_get_animation();