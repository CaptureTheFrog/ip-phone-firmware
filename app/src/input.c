//
// Created by eddie on 22/05/24.
//

#include "input.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdint.h>

int input_devices[MAX_INPUT_DEVICES];
size_t num_input_devices = 0;
int epollfd = -1;
struct epoll_event event_queue[MAX_EVENT_QUEUE];
void (*__event_callback)(struct input_event) = NULL;

int init_input_devices(void (*ec)(struct input_event)){
    __event_callback = ec;
    if (num_input_devices > 0){
        // TODO: unbind from existing devices
    }

    if(epollfd != -1){
        // TODO: close existing epoll fd
    }
    epollfd = epoll_create1(0);
    if(epollfd == -1){
        perror("Could not create input epoll");
        return EXIT_FAILURE;
    }

    DIR *d;
    struct dirent *dir;
    d = opendir(INPUT_DIR);
    if (d == NULL) {
        perror("Could not open input devices directory");
        close(epollfd);
        return EXIT_FAILURE;
    }

    while (num_input_devices < MAX_INPUT_DEVICES && (dir = readdir(d)) != NULL){
        if(strncmp(dir->d_name, EVENT_FILE_PREFIX, strlen(EVENT_FILE_PREFIX)) == 0){
            // this is a /dev/input/eventN
            // build the full file path
            char* path;
            if(asprintf(&path, "%s/%s", INPUT_DIR, dir->d_name) == -1){
                perror("Could not build input device file path");
                continue;
            }

            // if we want to check the device type, we could do that now
            // for now we will just use all input devices

            // open the event file
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                perror("Could not open input device");
                free(path);
                continue;
            }
            input_devices[num_input_devices++] = fd;
            free(path);

            // add to epoll
            struct epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = fd;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                perror("Could not add input device to epoll");
                continue;
            }

        }
    }
    closedir(d);
    return EXIT_SUCCESS;
}

int do_input_poll(int timeout){
    int nfds = epoll_wait(epollfd, event_queue, MAX_EVENT_QUEUE, -1);
    if (nfds == -1) {
        perror("Error during input epoll");
        return EXIT_FAILURE;
    }

    for (uint8_t n = 0; n < nfds; ++n) {
        struct input_event ie;
        // read from fd
        int result = read(event_queue[n].data.fd, &ie, sizeof(ie));
        if(result == -1){
            perror("Could not read from input device");
            continue;
        }else if(result != sizeof(ie)){
            perror("Could not read an entire event from input device");
            return EXIT_FAILURE;
        }

        // event read ok!
        if(__event_callback != NULL && (ie.type == EV_KEY || ie.type == EV_SW)){
            __event_callback(ie);
        }
    }

    return EXIT_SUCCESS;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
int do_input_poll_loop(){
    for(;;){
        int ret = do_input_poll(-1);
        if(ret != EXIT_SUCCESS){
            return ret;
        }
    }
}
#pragma clang diagnostic pop

void cleanup_input_devices(){
    close(epollfd);
    for (uint8_t n = 0; n < num_input_devices; ++n) {
        close(input_devices[n]);
    }
}

char keycode_to_printable_char(__u16 code){
    if(code >= KEY_1 && code <= KEY_0) {
        return '0' + ((code - KEY_1 + 1) % 10);
    }else if(code >= KEY_NUMERIC_0 && code <= KEY_NUMERIC_9) {
        return '0' + (code - KEY_NUMERIC_0);
    }else if(code == KEY_NUMERIC_POUND){
        return '#';
    }else if(code == KEY_NUMERIC_STAR || code == KEY_KPASTERISK){
        return '*';
    }else if(code >= KEY_NUMERIC_A && code <= KEY_NUMERIC_D){
        return 'A' + (code - KEY_NUMERIC_A);
    }else if(code == KEY_ENTER){
        return '\n';
    }else if(code == KEY_BACKSPACE){
        return '\b';
    }
    return '\0';
};