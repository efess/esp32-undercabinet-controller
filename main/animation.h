/*
 * animation.h
 *
 *  Created on: Jan 1, 2016
 *      Author: efess
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <stdint.h>
#include <stdbool.h>
#include "pixels.h"
#include "appstate.h"

// for random/test
#define ANIMATION_TYPE_COUNT 4
#define TRANSITION_TYPE_COUNT 2

#define TRANSITION_COUNT 2
#define ANIMATION_COUNT 8

typedef void (*AnimationSetup)(void *state, room_state_settings* settings);
typedef void (*TransitionSetup)(void *state, Pixels* pixels);
typedef void (*AnimationFrame)(void *state, Pixels* pixels);
typedef bool (*TransitionFrame)(void *state, Pixels* pixels, Pixels* prev_pixels);

typedef struct {
  uint8_t id;
  uint16_t frameDelay;
  AnimationSetup setupFn;
  AnimationFrame frameFn;
  const char* name;
} animation_t;

typedef struct {
  uint8_t id;
  
  TransitionSetup setupFn;
  TransitionFrame frameFn;
  const char* name;
} transition_t;

void animation_runFrame(Pixels *pixels);
void animation_changeState(app_state_t *appstate);
void animation_init();

#endif /* ANIMATION_H_ */
