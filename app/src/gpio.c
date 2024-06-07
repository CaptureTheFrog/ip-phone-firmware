//
// Created by eddie on 31/05/24.
//
#define APP_USE_GPIO
#include "gpio.h"

struct gpiod_chip* chip;
struct gpiod_line* handset_mic_request_line;
struct gpiod_line* handset_mic_active_line;

void init_gpio(){
    chip = gpiod_chip_open_by_name("gpiochip3");
    handset_mic_request_line = gpiod_chip_get_line(chip, 4);
    handset_mic_active_line = gpiod_chip_get_line(chip, 5);
    gpiod_line_request_output(handset_mic_request_line, "IP Phone: " __FILE__, 0);
    gpiod_line_request_input(handset_mic_active_line, "IP Phone: " __FILE__);
}

void cleanup_gpio(){
    gpiod_line_release(handset_mic_request_line);
    gpiod_line_release(handset_mic_active_line);
    gpiod_chip_close(chip);
}