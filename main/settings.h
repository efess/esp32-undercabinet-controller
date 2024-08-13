/*
 * settings.h
 *
 *  Created on: Jan 4, 2016
 *      Author: efess
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>
#include "math.h"

#define MOTION_CONTROL_DEVICE 			  0
#define MOTION_CONTROL_HOME_ASSISTANCE    1

typedef struct {
	uint8_t brightness;
	uint8_t transition;
	uint8_t animation; 
	uint32_t color;
	uint32_t colorPallete[16];
} room_state_settings;

typedef struct lighting_settings {
	uint8_t motion_control;
	uint16_t occupiedTimeout; // seconds
	uint8_t alwaysOn;
	room_state_settings occupied;
	room_state_settings unoccupied;
} lighting_settings;


uint8_t settings_getBrightness(uint8_t settingsBrightness);
void init_settings();
void settings_update(const char* sender, lighting_settings* newSettings);

#endif /* SETTINGS_H_ */
