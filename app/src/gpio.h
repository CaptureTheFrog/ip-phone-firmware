//
// Created by eddie on 31/05/24.
//

#ifndef APP_GPIO_H
#define APP_GPIO_H
#ifdef APP_USE_GPIO
#include <gpiod.h>

extern struct gpiod_chip* chip;
extern struct gpiod_line* handset_mic_request_line;
extern struct gpiod_line* handset_mic_active_line;
void init_gpio();
void cleanup_gpio();

#endif /* APP_USE_GPIO */
#endif //APP_GPIO_H
