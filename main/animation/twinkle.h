/*
 * twinkle.h
 *
 *  Created on: Jan 4, 2016
 *      Author: efess
 */

#include <stdint.h>
#include "../animation.h"
#include "../appstate.h"
#include "../settings.h"


typedef struct {
	uint16_t lifeCounter;
	uint8_t options;
} Twinkle;

typedef struct {
	Twinkle twinkle[LED_COUNT];
	uint16_t frameNum;

} TwinkleState;

animation_t* twinkle_get_animation();