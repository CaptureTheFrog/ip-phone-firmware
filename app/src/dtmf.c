//
// Created by eddie on 22/05/24.
//


#include <pulse/simple.h>
#include <pulse/error.h>
#include "dtmf.h"
#include <math.h>
#include <endian.h>
#include <stdio.h>
#include <string.h>

#define BUF_SAMPLES    512
#define BUF_MS  (((double)BUF_SAMPLES / dtmf_ss.rate) * 1000.0)
#define SAMPLE_MS   (((double)1 / dtmf_ss.rate) * 1000.0)

#define DTMF_FADE_IN_MS 2.0
#define DTMF_FADE_OUT_MS 2.0

#include <stdlib.h>
#include <pthread.h>

static const pa_sample_spec dtmf_ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 1
};

pa_simple* dtmf_pa_connection = NULL;
pthread_mutex_t dtmf_mutex = PTHREAD_MUTEX_INITIALIZER;
int background_dtmf = 0;
pthread_t background_dtmf_thread;
volatile int request_stop_dtmf = 0;

int char_to_dtmf_freqs(char c, uint16_t* freq1, uint16_t* freq2){
    switch(c){
        case '0':
            *freq1 = DTMF_DIGIT_0_F1;
            *freq2 = DTMF_DIGIT_0_F2;
            return EXIT_SUCCESS;
        case '1':
            *freq1 = DTMF_DIGIT_1_F1;
            *freq2 = DTMF_DIGIT_1_F2;
            return EXIT_SUCCESS;
        case '2':
            *freq1 = DTMF_DIGIT_2_F1;
            *freq2 = DTMF_DIGIT_2_F2;
            return EXIT_SUCCESS;
        case '3':
            *freq1 = DTMF_DIGIT_3_F1;
            *freq2 = DTMF_DIGIT_3_F2;
            return EXIT_SUCCESS;
        case '4':
            *freq1 = DTMF_DIGIT_4_F1;
            *freq2 = DTMF_DIGIT_4_F2;
            return EXIT_SUCCESS;
        case '5':
            *freq1 = DTMF_DIGIT_5_F1;
            *freq2 = DTMF_DIGIT_5_F2;
            return EXIT_SUCCESS;
        case '6':
            *freq1 = DTMF_DIGIT_6_F1;
            *freq2 = DTMF_DIGIT_6_F2;
            return EXIT_SUCCESS;
        case '7':
            *freq1 = DTMF_DIGIT_7_F1;
            *freq2 = DTMF_DIGIT_7_F2;
            return EXIT_SUCCESS;
        case '8':
            *freq1 = DTMF_DIGIT_8_F1;
            *freq2 = DTMF_DIGIT_8_F2;
            return EXIT_SUCCESS;
        case '9':
            *freq1 = DTMF_DIGIT_9_F1;
            *freq2 = DTMF_DIGIT_9_F2;
            return EXIT_SUCCESS;
        case '*':
            *freq1 = DTMF_DIGIT_STAR_F1;
            *freq2 = DTMF_DIGIT_STAR_F2;
            return EXIT_SUCCESS;
        case '#':
            *freq1 = DTMF_DIGIT_HASH_F1;
            *freq2 = DTMF_DIGIT_HASH_F2;
            return EXIT_SUCCESS;
        case 'A':
            *freq1 = DTMF_DIGIT_A_F1;
            *freq2 = DTMF_DIGIT_A_F2;
            return EXIT_SUCCESS;
        case 'B':
            *freq1 = DTMF_DIGIT_B_F1;
            *freq2 = DTMF_DIGIT_B_F2;
            return EXIT_SUCCESS;
        case 'C':
            *freq1 = DTMF_DIGIT_C_F1;
            *freq2 = DTMF_DIGIT_C_F2;
            return EXIT_SUCCESS;
        case 'D':
            *freq1 = DTMF_DIGIT_D_F1;
            *freq2 = DTMF_DIGIT_D_F2;
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
    }
}

int init_dtmf(){
    cleanup_dtmf(); // just in case we already had a connection

    int error;
    if (!(dtmf_pa_connection = pa_simple_new(NULL, "IP Phone", PA_STREAM_PLAYBACK, NULL, "DTMF", &dtmf_ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return EXIT_FAILURE;
    }
}

void dtmf_drain(){
    int error;
    if (pa_simple_drain(dtmf_pa_connection, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
    }
};

void __dtmf(struct __dtmf_args* a){
    int16_t buf[BUF_SAMPLES];
    double phase1 = 0, phase2 = 0;
    double phase_inc1 = (2.0 * M_PI * a->freq1) / dtmf_ss.rate;
    double phase_inc2 = (2.0 * M_PI * a->freq2) / dtmf_ss.rate;
    double amplitude_max = 1.0;
    double amplitude = 0.0;
    double fade_in_ms = DTMF_FADE_IN_MS;
    double fade_out_ms = DTMF_FADE_OUT_MS;
    int bg = (a->on_time_ms == -1.0);

    pthread_mutex_lock(&dtmf_mutex);
    // Generate and play tone indefinitely
    while(
            (bg && !request_stop_dtmf) ||
            (a->on_time_ms > 0 && !request_stop_dtmf) ||
            (bg && (a->on_time_ms > 0 || (a->on_time_ms == -1.0 && ({a->on_time_ms = DTMF_FADE_OUT_MS; 1;})))) ||
            (!bg && a->on_time_ms > 0 && (a->on_time_ms <= DTMF_FADE_OUT_MS || ({a->on_time_ms = MIN(DTMF_FADE_OUT_MS,a->on_time_ms); 1;})))){
        size_t i;
        if (a->freq1 == 0 && a->freq2 == 0) {
            memset(buf, 0, sizeof(buf));
            a->on_time_ms = MAX(a->on_time_ms - BUF_MS, 0);
            i = sizeof(buf) / sizeof(buf[0]);
        } else {
            for (i = 0; i < BUF_SAMPLES; i++) {
                if(fade_in_ms > 0){
                    amplitude = amplitude_max * pow((1.0-(fade_in_ms/DTMF_FADE_IN_MS)), M_E);
                    fade_in_ms -= SAMPLE_MS;
                    if(fade_in_ms <= 0.0)
                        amplitude = amplitude_max;
                }
                if(a->on_time_ms > 0 && a->on_time_ms <= DTMF_FADE_OUT_MS){
                    //printf("%f\n", amplitude);
                    if(fade_out_ms == 0.0)
                        break;
                    amplitude = amplitude_max * pow((fade_out_ms/DTMF_FADE_OUT_MS), M_E);
                    fade_out_ms = MAX(fade_out_ms - SAMPLE_MS, 0.0);
                }
                int16_t v = (int16_t)((sin(phase1) + sin(phase2)) * 16383.0 * amplitude * ((a->freq1 == 0 || a->freq2 == 0) ? 2 : 1));
#if __BYTE_ORDER == __LITTLE_ENDIAN
                buf[i] = v;
#elif __BYTE_ORDER == __BIG_ENDIAN
                // Swap endianness of sample
                buf[i] = (v << 8) | ((v >> 8) & 0xFF);
#else
#error "Endianness not set!"
#endif

                if(a->on_time_ms != -1.0)
                    a->on_time_ms = MAX(a->on_time_ms - SAMPLE_MS, 0);
                phase1 += phase_inc1;
                phase2 += phase_inc2;
                if (phase1 >= 2.0 * M_PI) phase1 -= 2.0 * M_PI;
                if (phase2 >= 2.0 * M_PI) phase2 -= 2.0 * M_PI;
            }
        }
        int error;

        //printf("%lu, %lu\n", i, i * sizeof(buf[0]));
        /* Play the buffer */
        if (i > 0 && pa_simple_write(dtmf_pa_connection, buf, i * sizeof(buf[0]), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
        }
    }
    if (bg == 1){
        dtmf_drain();
    }
    pthread_mutex_unlock(&dtmf_mutex);
    free(a);
}

void dtmf_stop_bg(){
    request_stop_dtmf = 1;
    fprintf(stderr, "offplz...");
    if(background_dtmf){
        pthread_join(background_dtmf_thread, NULL);
        background_dtmf = 0;
    }else{
        pthread_mutex_lock(&dtmf_mutex);
        pthread_mutex_unlock(&dtmf_mutex);
    }
    fprintf(stderr, "off\n");
    request_stop_dtmf = 0;
}

void dtmf(uint16_t freq1, uint16_t freq2, double on_time_ms){
    struct __dtmf_args* d = malloc(sizeof(struct __dtmf_args));
    d->freq1 = freq1;
    d->freq2 = freq2;
    d->on_time_ms = on_time_ms;
    dtmf_stop_bg();
    if(on_time_ms == -1){
        // indefinite length, so new thread
        background_dtmf = 1;
        int err = pthread_create(&background_dtmf_thread, NULL, (void*(*)(void *))__dtmf, d);
        if(err != 0){
            fprintf(stderr, "Error in pthread create: %x\n", err);
        }
    }else{
        __dtmf(d);
    }
}

void cleanup_dtmf(){
    if (dtmf_pa_connection) {
        pa_simple_free(dtmf_pa_connection);
    }
}