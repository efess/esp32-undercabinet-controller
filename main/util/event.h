#include "esp_wifi_types.h"


#define EVENT_MOTION_STATE_CHANGED "motion_state_changed"
#define EVENT_LIGHT_SETTINGS_UPDATED "light_settings_changed"

typedef void (*event_callback)(const char* sender, const void *data);

struct event {
    const char *name;
    event_callback callback;
    struct event *next;
};

typedef struct event event;

void event_add_handler(const char* event_name, event_callback callback);
void event_trigger(const char* sender, const char* event_name, const void *data);
