#include <stdlib.h>
#include "esp_heap_trace.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "string.h"
#include "animation.h"
#include "transition/fade.h"
#include "transition/pixelate.h"
#include "animation/normal.h"
#include "animation/twinkle.h"
#include "animation/rainbow.h"
#include "animation/runner.h"
#include "animation/motion.h"
#include "animation/discrete.h"
#include "animation/fire.h"
#include "animation/freqs.h"

#define ANIMATION_STATE_SIZE 1024

#define ANIMATION_NONE 0
#define ANIMATION_MOTION 1
#define ANIMATION_TWINKLE 2
#define ANIMATION_RAINBOW 3
#define ANIMATION_RUNNER 4
#define ANIMATION_DISCRETE 5
#define ANIMATION_FIRE 6
#define ANIMATION_FREQS 7

#define TRANSITION_FADE 1
#define TRANSITION_PIXELATE 2

#define TAG "animation"

typedef enum {
  NONE,
  MOTION,
  TWINKLE,
  RAINBOW,
  RUNNER,
  DISCRETE,
  FIRE,
  FREQS
} animation_type_t;

animation_t* animations[ANIMATION_COUNT];
transition_t* transitions[TRANSITION_COUNT];

animation_t* prevAnimation;
animation_t* currentAnimation;
transition_t* currentTransition;

Pixels _prev_animation_pixels;

void *_prev_animation_state;
void *_animation_state;
void *_transition_state;

uint16_t testCounter;

void animation_runFrame(Pixels *pixels)
{
	// test(pixels);

	if(prevAnimation)
	{
		prevAnimation->frameFn(_prev_animation_state, &_prev_animation_pixels);
	}
	if(currentAnimation)
	{
		currentAnimation->frameFn(_animation_state, pixels);
	}
	if(currentTransition)
	{
		if(currentTransition->frameFn(_transition_state, pixels, &_prev_animation_pixels))
		{
			currentTransition = NULL;
      prevAnimation = NULL;
		}
	}
}


void changeAnimation(
		Pixels *pixels,
		room_state_settings *settings,
		transition_t *transition,
		animation_t *animation)
{
  if (currentAnimation && transition){
    prevAnimation = currentAnimation;
    memcpy(_prev_animation_pixels.pixelData, pixels->pixelData, LED_BYTE_COUNT);
    memcpy(_prev_animation_state, _animation_state, ANIMATION_STATE_SIZE);
  }

  if(transition && transition->setupFn)
  {
    transition->setupFn(_transition_state, pixels);
  }

  if(animation->setupFn)
  {
    animation->setupFn(_animation_state, settings);
  }

  currentAnimation = animation;
  currentTransition = transition;
}

void animation_changeState(app_state_t *appstate)
{
	room_state_settings* roomSettings = appstate->isOccupied ?
			&(appstate->settings->occupied) :
			&(appstate->settings->unoccupied);

	transition_t *transition = NULL;
	animation_t *animation = NULL;

  for(uint8_t i = 0; i < TRANSITION_COUNT; i++) {
    if (transitions[i]->id == roomSettings->transition) {
      ESP_LOGI(TAG, "Transaction #%d has id of %d", i, roomSettings->transition);
      transition = transitions[i];
      break;
    }
  }
  for(uint8_t i = 0; i < ANIMATION_COUNT; i++) {
    if (animations[i]->id == roomSettings->animation) {
      ESP_LOGI(TAG, "Animation #%d has id of %d", i, roomSettings->animation);
      animation = animations[i];
      break;
    }
  }
  if (!transition) {
    ESP_LOGI(TAG, "No Transition selected.");
    transition = NULL;
  }
  if (!animation) {
    ESP_LOGI(TAG, "Animation not found, using default.");
    animation = animations[0];
  }

	changeAnimation(appstate->pixels, roomSettings, transition, animation);
}

void test(Pixels *pixels)
{
	lighting_settings settings;
	settings.occupied.animation = 1;
	settings.occupied.transition = 1;
	settings.occupied.brightness = 8;

	settings.unoccupied.animation = 2;
	settings.unoccupied.transition = 1;
	settings.unoccupied.brightness = 3;

	if(testCounter % 500 == 0)
	{
		uint8_t rndTrans = rand() % TRANSITION_COUNT;
		uint8_t rndAnim = rand() % ANIMATION_COUNT;

		changeAnimation(pixels,
				&settings.occupied,
				transitions[rndTrans],
				animations[rndAnim]);
	}

	testCounter++;
}

void animation_init()
{
  prevAnimation = NULL;
  currentTransition = NULL;
  currentAnimation = NULL;

  _prev_animation_pixels.pixelCount = LED_COUNT;
  _prev_animation_pixels.pixelData = malloc(LED_BYTE_COUNT);

  _prev_animation_state = malloc(ANIMATION_STATE_SIZE); // 1k allocated for transitiong from animation state
  _animation_state = malloc(ANIMATION_STATE_SIZE); // 1k allocated for animation state
  _transition_state = malloc(ANIMATION_STATE_SIZE); // 1k allocated for animation state

  animations[0] = normal_get_animation();
  animations[1] = motion_get_animation();
  animations[2] = twinkle_get_animation();
  animations[3] = rainbow_get_animation();
  animations[4] = runner_get_animation();
  animations[5] = discrete_get_animation();
  animations[6] = fire_get_animation();
  animations[7] = freqs_get_animation();

  transitions[0] = fade_get_transition();
  transitions[1] = pixelate_get_transition();
}
