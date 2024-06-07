//
// Created by eddie on 23/05/24.
//

#ifndef APP_SIP_H
#define APP_SIP_H

#define THIS_FILE       "APP"

#include "sip_secrets.h"

int sip_is_line_active();
void sip_send_dtmf(char digit);
void init_sip();
void register_sip();
void cleanup_sip();
void sip_dial(char* dialout_digits);
int sip_answer();
int sip_hangup();

#endif //APP_SIP_H
