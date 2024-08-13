/*
 * normal.h
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
} NormalState;

animation_t* normal_get_animation();