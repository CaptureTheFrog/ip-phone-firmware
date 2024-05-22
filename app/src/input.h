//
// Created by eddie on 22/05/24.
//

#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <linux/input.h>

#ifndef MAX_INPUT_DEVICES
#define MAX_INPUT_DEVICES   16
#endif //MAX_INPUT_DEVICES

#define MAX_EVENT_QUEUE 64

#define INPUT_DIR   "/dev/input"
#define EVENT_FILE_PREFIX   "event"

int init_input_devices(void (*event_callback)(struct input_event));
int do_input_poll(int timeout);
int do_input_poll_loop();

#endif //APP_INPUT_H
