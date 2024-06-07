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

#define APP_KEYPAD_TERMINATE_CALL   KEY_HANGUP_PHONE
#define APP_KEYPAD_ACCEPT_CALL      KEY_PICKUP_PHONE
#define APP_KEYPAD_1                KEY_NUMERIC_1
#define APP_KEYPAD_2                KEY_NUMERIC_2
#define APP_KEYPAD_3                KEY_NUMERIC_3
#define APP_KEYPAD_4                KEY_NUMERIC_4
#define APP_KEYPAD_5                KEY_NUMERIC_5
#define APP_KEYPAD_6                KEY_NUMERIC_6
#define APP_KEYPAD_7                KEY_NUMERIC_7
#define APP_KEYPAD_8                KEY_NUMERIC_8
#define APP_KEYPAD_9                KEY_NUMERIC_9
#define APP_KEYPAD_0                KEY_NUMERIC_0
#define APP_KEYPAD_STAR             KEY_NUMERIC_STAR
#define APP_KEYPAD_HASH             KEY_NUMERIC_POUND

#define APP_KEYPAD_DPAD_UP          BTN_DPAD_UP
#define APP_KEYPAD_DPAD_DOWN        BTN_DPAD_DOWN
#define APP_KEYPAD_DPAD_LEFT        BTN_DPAD_LEFT
#define APP_KEYPAD_DPAD_RIGHT       BTN_DPAD_RIGHT
#define APP_KEYPAD_VOL_UP           KEY_VOLUMEUP
#define APP_KEYPAD_VOL_DOWN         KEY_VOLUMEDOWN
#define APP_KEYPAD_MENU             KEY_MENU
#define APP_KEYPAD_SPEAKERPHONE     KEY_PHONE
#define APP_KEYPAD_OK               KEY_OK
#define APP_KEYPAD_PHONEBOOK        KEY_ADDRESSBOOK
#define APP_KEYPAD_VOICEMAIL        KEY_VOICEMAIL
#define APP_KEYPAD_CANCEL           KEY_CANCEL
#define APP_KEYPAD_REDIAL           KEY_REDO

#define APP_SWITCH_MIC_MUTE         SW_MUTE_DEVICE
#define APP_SWITCH_HOOK             SW_MICROPHONE_INSERT


int init_input_devices(void (*event_callback)(struct input_event));
int do_input_poll(int timeout);
int do_input_poll_loop();
void cleanup_input_devices();
char keycode_to_printable_char(__u16 code);

#endif //APP_INPUT_H
