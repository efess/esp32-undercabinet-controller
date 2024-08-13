 /*
 * fire.h
 *
 *  Created on: Jan 17, 2016
 *      Author: efess
 */

#include <stdint.h>
#include <stdbool.h>
#include "../animation.h"
#include "../appstate.h"
#include "../settings.h"

#define FIRE_DIFFUSE 10

typedef struct {
	bool isFlareUp;
	uint8_t fireIdx;
} FirePixel;

typedef struct {
	uint8_t brightness;
	FirePixel fire[LED_COUNT];
	uint32_t frameNum;
} FireState;

animation_t* fire_get_animation();
