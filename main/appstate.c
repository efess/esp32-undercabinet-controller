#include "appstate.h"
#include "string.h"

app_state_t appstate;

void appstate_init() {
	appstate.settings = (lighting_settings*)malloc(sizeof(lighting_settings));
	appstate.pixels = (Pixels*)malloc(sizeof(Pixels));
	appstate.pixels->pixelCount = LED_COUNT;
	appstate.pixels->pixelData = malloc(LED_BYTE_COUNT); // Buffer before sending
	memset(appstate.pixels->pixelData, 0, LED_BYTE_COUNT);
}
