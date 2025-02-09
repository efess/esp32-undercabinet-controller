#include "motion.h"
#include <stdlib.h>
#include <stdbool.h>
#include "string.h"
#include "math.h"
#include "../appstate.h"
#include "../util/rgbfunc.h"
#include "../sound.h"

int8_t getRandomNodeIdx(MotionState *state, uint8_t idx)
{
	uint8_t i;
	for(i = 0; i < RANDOM_NODE_SIZE; i++)
	{
		if(state->rndNodeIndex[i] == idx)
		{
			return i;
		}
	}
	return -1; // not found
}

void motion_setup(void *g_state, room_state_settings* settings)
{
	MotionState *state = g_state;

	memset(state, 0, sizeof(MotionState));
	state->brightness = settings_getBrightness(settings->brightness);

	uint16_t i = 0;
	uint8_t node_spacing = LED_COUNT / (PALLETE_SIZE - 1);
	for(i = 0; i < RANDOM_NODE_SIZE; i++)
	{
		state->rndNodeIndex[i] = rand() % 4 + (i*4);
	}
	for(i = 0; i < RANDOM_NODE_SIZE; i++)
	{
		state->rndNodeFrequency[i] = rand() % 100 + 100;
	}

    for(i = 0; i < PALLETE_SIZE; i++)
	{
        // set start locations
    	state->nodeStart[i] = node_spacing * (i + 1);
    	int8_t rIdx = getRandomNodeIdx(state, i);
    	if(rIdx > -1)
    	{
    		state->rndNodeStart[rIdx] = state->nodeStart[i];
    	}
    }

    memcpy(state->nodeColor, settings->colorPallete, sizeof(settings->colorPallete));
}

void motion_frame(void *g_state, Pixels* pixels)
{
	uint8_t diffuseNodeWidth = fminf(NODE_DIFFUSE_WIDTH, pixels->pixelCount);
	MotionState *state = g_state;
	uint16_t i;
	for(i = 0; i < RANDOM_NODE_SIZE; i++)
	{
		uint16_t offset = floorf( sinf( (float)state->frameNum / (float)state->rndNodeFrequency[i]) * 90.0);
		state->nodeStart[state->rndNodeIndex[i]] = state->rndNodeStart[i] + offset;
	}

	uint8_t stretch = (uint32_t)floorf(state->frameNum / 6.0) % pixels->pixelCount;
	uint16_t k, pos;
	uint32_t nextColor;
	int8_t j;

	for(i = 0; i < PALLETE_SIZE; i++)
	{

		pos = (uint16_t)state->nodeStart[i] + (uint16_t)stretch;

		for(j = -diffuseNodeWidth + 1; j < diffuseNodeWidth; j++) {
			k = (j + pos + pixels->pixelCount) % pixels->pixelCount;

			nextColor = blendA(pixels_getPixelColor(pixels, k), state->nodeColor[i], ((float)abs(j) / (float)NODE_DIFFUSE_WIDTH) * 255);

			pixels_setPixelColor(pixels, k, changeBrightness(nextColor, state->brightness));
		}
	}
	state->frameNum ++;
}

animation_t motionAnimation = {
  .id = 1,
  .frameDelay = 20,
  .setupFn = motion_setup,
  .frameFn = motion_frame,
  .name = "Motion"
};

animation_t* motion_get_animation() {
	return &motionAnimation;
}