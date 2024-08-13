/*
 * pixelate.h
 *
 *  Created on: Jan 3, 2016
 *      Author: efess
 */

#include <stdint.h>
#include "../animation.h"
#include "../appstate.h"

typedef struct {
	uint16_t frameNum;
	uint8_t pixelated[50];

} PixelateState;

transition_t* pixelate_get_transition();