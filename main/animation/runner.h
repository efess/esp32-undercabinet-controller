/*
 * runner.h
 *
 *  Created on: Jan 3, 2016
 *      Author: efess
 */

#include <stdint.h>
#include "../animation.h"
#include "../pixels.h"
#include "../settings.h"

typedef struct {
	uint32_t color;
	uint8_t brightness;
	uint16_t position;
} RunnerState;

animation_t* runner_get_animation();