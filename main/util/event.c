#include "string.h"
#include "./event.h"

event *events = NULL;

void event_add_handler(const char* event_name, event_callback callback) {
    event *evnt = malloc(sizeof(event));
    evnt->name = event_name;
    evnt->callback = callback;
    if (events) {
        evnt->next = events;
    } else {
        evnt->next = NULL;
    }

    events = evnt;
}

void event_trigger(const char* sender, const char* event_name, const void *data) {
    event *evnt = events;
    while (evnt) {
        if (strcmp(evnt->name, event_name) == 0) {
            evnt->callback(sender, data);
        }
        evnt = evnt->next;
    }
}