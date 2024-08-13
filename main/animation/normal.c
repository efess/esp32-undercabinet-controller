/*
 * normal.c
 *
 *  Created on: Jan 11, 2016
 *      Author: efess
 */

#include "normal.h"
#include "string.h"
#include "math.h"
#include "../sound.h"
#include "../util/rgbfunc.h"

void normal_setup(void *g_state, room_state_settings* settings)
{
	NormalState *state = g_state;

	memset(state, 0, sizeof(NormalState));
	state->brightness = settings_getBrightness(settings->brightness);
	state->color = settings->color;
}

void normal_frame(void *g_state, Pixels* pixels)
{
	NormalState *state = g_state;

	uint16_t i;
	for(i = 0; i < pixels->pixelCount; i++)
	{
		pixels_setPixelColor(pixels, i, changeBrightness(state->color, state->brightness));//state->brightness));
    }
}

animation_t normalAnimation = {
  .id = 0,
  .frameDelay = 20,
  .setupFn = normal_setup,
  .frameFn = normal_frame,
  .name = "Strait"
};

animation_t* normal_get_animation() {
	return &normalAnimation;
}