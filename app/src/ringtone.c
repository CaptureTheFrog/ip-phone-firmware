//
// Created by eddie on 24/05/24.
//

#include "ringtone.h"
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdlib.h>
#include <pthread.h>

#define RINGTONE_NAME   "simple"

#define RINGTONE_BUFFER_SAMPLES 128
#define RINGTONE_FADE_OUT_MS    1.0

static const pa_sample_spec ringtone_ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
};

pa_simple* ringtone_pa_connection = NULL;
pthread_mutex_t ringtone_mutex = PTHREAD_MUTEX_INITIALIZER;
void* ringtone_samples;
size_t ringtone_samples_bytes;
pthread_t ringtone_thread;

volatile int request_ringtone_stop = 0;

int init_ringtone(){
    cleanup_ringtone(); // just in case we already had a connection

    int error;
    if (!(ringtone_pa_connection = pa_simple_new(NULL, "IP Phone", PA_STREAM_PLAYBACK, NULL, "Ringtone", &ringtone_ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return EXIT_FAILURE;
    }

    int fd = open("../media/ringtones/" RINGTONE_NAME ".raw", O_RDONLY);
    if(fd == -1){
        perror("init_ringtone() open()");
        return EXIT_FAILURE;
    }

    struct stat s;
    if(fstat(fd, &s) == -1){
        perror("init_ringtone() fstat()");
        return EXIT_FAILURE;
    }

    ringtone_samples = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(ringtone_samples == (void*)-1){
        perror("init_ringtone() mmap()");
        return EXIT_FAILURE;
    }
    ringtone_samples_bytes = s.st_size;
    return EXIT_SUCCESS;
}

void ringtone_on(){
    if(pthread_mutex_trylock(&ringtone_mutex) == 0){
        pthread_mutex_unlock(&ringtone_mutex);
        int err = pthread_create(&ringtone_thread, NULL, (void*(*)(void *))play_ringtone, NULL);
        if(err != 0){
            fprintf(stderr, "Error in pthread create: %x\n", err);
        }
    }
}

void ringtone_off(){
    if(pthread_mutex_trylock(&ringtone_mutex) == 0) {
        pthread_mutex_unlock(&ringtone_mutex);
    }else{
        request_ringtone_stop = 1;
    }
}

static void play_ringtone(){
    pthread_mutex_lock(&ringtone_mutex);
    int error;
    while(!request_ringtone_stop){
        for(size_t i=0; i < (ringtone_samples_bytes+1)/RINGTONE_BUFFER_SAMPLES; i++)
        if (pa_simple_write(ringtone_pa_connection, ringtone_samples + (i * RINGTONE_BUFFER_SAMPLES), MIN(RINGTONE_BUFFER_SAMPLES, ringtone_samples_bytes - (i * RINGTONE_BUFFER_SAMPLES)), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
        }
    }
    request_ringtone_stop = 0;
    pthread_mutex_unlock(&ringtone_mutex);
}

void cleanup_ringtone(){
    if (ringtone_pa_connection) {
        //dtmf_stop_bg();
        //dtmf_drain();
        munmap(ringtone_samples, ringtone_samples_bytes);
        pa_simple_free(ringtone_pa_connection);
    }
}