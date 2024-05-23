//
// Created by eddie on 22/05/24.
//

#ifndef APP_DTMF_H
#define APP_DTMF_H

#define DTMF_DIGIT_1_F1	697
#define DTMF_DIGIT_1_F2	1209
#define DTMF_DIGIT_2_F1	697
#define DTMF_DIGIT_2_F2	1336
#define DTMF_DIGIT_3_F1	697
#define DTMF_DIGIT_3_F2	1477
#define DTMF_DIGIT_A_F1	697
#define DTMF_DIGIT_A_F2	1633
#define DTMF_DIGIT_4_F1	770
#define DTMF_DIGIT_4_F2	1209
#define DTMF_DIGIT_5_F1	770
#define DTMF_DIGIT_5_F2	1336
#define DTMF_DIGIT_6_F1	770
#define DTMF_DIGIT_6_F2	1477
#define DTMF_DIGIT_B_F1	770
#define DTMF_DIGIT_B_F2	1633
#define DTMF_DIGIT_7_F1	852
#define DTMF_DIGIT_7_F2	1209
#define DTMF_DIGIT_8_F1	852
#define DTMF_DIGIT_8_F2	1336
#define DTMF_DIGIT_9_F1	852
#define DTMF_DIGIT_9_F2	1477
#define DTMF_DIGIT_C_F1	852
#define DTMF_DIGIT_C_F2	1633
#define DTMF_DIGIT_STAR_F1	941
#define DTMF_DIGIT_STAR_F2	1209
#define DTMF_DIGIT_0_F1	941
#define DTMF_DIGIT_0_F2	1336
#define DTMF_DIGIT_HASH_F1	941
#define DTMF_DIGIT_HASH_F2	1477
#define DTMF_DIGIT_D_F1	941
#define DTMF_DIGIT_D_F2	1633

#define DTMF_UK_DIALTONE_F1 350
#define DTMF_UK_DIALTONE_F2 450

#define DTMF_UK_RINGING_F1  400
#define DTMF_UK_RINGING_F2  450
#define DTMF_UK_RINGING_CADENCE_ON  400
#define DTMF_UK_RINGING_CADENCE_OFF 200
#define DTMF_UK_RINGING_CADENCE_COUNT   2
#define DTMF_UK_RINGING_CADENCE_PAUSE 2000

#include <stdint.h>

struct __dtmf_args{
    uint16_t freq1;
    uint16_t freq2;
    double on_time_ms;
};

void __dtmf(struct __dtmf_args* a);

int char_to_dtmf_freqs(char c, uint16_t* freq1, uint16_t* freq2);
void dtmf(uint16_t freq1, uint16_t freq2, double on_time_ms);
int init_dtmf();
void cleanup_dtmf();
void dtmf_stop_bg();
void dtmf_drain();


#endif //APP_DTMF_H
