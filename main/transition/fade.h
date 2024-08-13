/*
 * fade.h
 *
 *  Created on: Jan 1, 2016
 *      Author: efess
 */

#include <stdbool.h>
#include "../animation.h"
#include "../appstate.h"

typedef struct {
	uint8_t fadeAmount;
} FadeState;

transition_t* fade_get_transition();