/*
 * appstate.h
 *
 *  Created on: Feb 3, 2016
 *      Author: efess
 */

#ifndef APPSTATE_H_
#define APPSTATE_H_

#include <stdbool.h>
#include "pixels.h"
#include "settings.h"

// #define LED_COUNT 231
#define LED_COUNT 231
#define LED_BYTE_COUNT LED_COUNT * 3

typedef struct {
	float frequencies[16];
	Pixels *pixels;
	lighting_settings *settings;
	bool isOccupied;
} app_state_t;

void appstate_init();

#endif /* APPSTATE_H_ */
