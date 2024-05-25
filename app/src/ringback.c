//
// Created by eddie on 25/05/24.
//

#include "ringback.h"
#include "dtmf.h"
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t ringback_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t ringback_thread;
volatile int request_ringback_stop = 0;

static void play_ringback(){
    pthread_mutex_lock(&ringback_mutex);
    while(!request_ringback_stop){
        for(int i=0; i<DTMF_UK_RINGBACK_CADENCE_COUNT; i++){
            dtmf(DTMF_UK_RINGBACK_F1, DTMF_UK_RINGBACK_F2, DTMF_UK_RINGBACK_CADENCE_ON);
            dtmf(0, 0, DTMF_UK_RINGBACK_CADENCE_OFF);
        }
        dtmf(0, 0, DTMF_UK_RINGBACK_CADENCE_PAUSE);
    }
    request_ringback_stop = 0;
    pthread_mutex_unlock(&ringback_mutex);
}

void ringback_on(){
    if(pthread_mutex_trylock(&ringback_mutex) == 0){
        pthread_mutex_unlock(&ringback_mutex);
        int err = pthread_create(&ringback_thread, NULL, (void*(*)(void *))play_ringback, NULL);
        if(err != 0){
            fprintf(stderr, "Error in pthread create: %x\n", err);
        }
    }
}

void ringback_off(){
    if(pthread_mutex_trylock(&ringback_mutex) == 0) {
        pthread_mutex_unlock(&ringback_mutex);
    }else{
        request_ringback_stop = 1;
    }
}