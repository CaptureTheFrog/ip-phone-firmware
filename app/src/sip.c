//
// Created by eddie on 23/05/24.
//

#include "sip.h"
#include "ringtone.h"
#include "ringback.h"

#include <pjsua-lib/pjsua.h>

pjsua_acc_id sip_acc_id;
pjsua_call_id sip_active_call_id;
volatile static int sip_line_active = 0;

int sip_is_line_active(){
    return sip_line_active;
}

void sip_send_dtmf(char digit){
    char d[2];
    d[0] = digit;
    d[1] = '\0';

    pjsua_call_send_dtmf_param p;
    pjsua_call_send_dtmf_param_default(&p);
    p.digits = pj_str(d);

    pjsua_call_send_dtmf(sip_active_call_id, &p);
}

/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
            (int)ci.remote_info.slen,
            ci.remote_info.ptr));

    if(sip_line_active){
        // busy here
        fprintf(stderr, "Rejecting call from %.*s, we are busy\n", (int)ci.remote_info.slen,
                ci.remote_info.ptr);
        pjsua_call_hangup(call_id, 486, NULL, NULL);
        return;
    }

    sip_line_active = 1;
    sip_active_call_id = call_id;

    ringtone_on();
    fprintf(stderr, "Incoming Call from %.*s\n", (int)ci.remote_info.slen,
            ci.remote_info.ptr);

    /* Automatically answer incoming calls with 200/OK */
    //pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
            (int)ci.state_text.slen,
            ci.state_text.ptr));

    if(sip_line_active && sip_active_call_id == call_id && ci.state != PJSIP_INV_STATE_INCOMING)
        ringtone_off();

    switch(ci.state){
        case PJSIP_INV_STATE_INCOMING:
            fprintf(stderr, "Incoming Call\n");
            break;
        case PJSIP_INV_STATE_CONFIRMED:
            fprintf(stderr, "Connected\n");
            ringback_off();
            break;
        case PJSIP_INV_STATE_DISCONNECTED:
            fprintf(stderr, "Disconnected\n");
            ringback_off();
            if(sip_line_active && sip_active_call_id == call_id) sip_line_active = 0;
            break;
        case PJSIP_INV_STATE_CALLING:
            fprintf(stderr, "Calling\n");
            sip_line_active = 1;
            sip_active_call_id = call_id;
            break;
        case PJSIP_INV_STATE_EARLY:
            ringback_on();
            break;
    }
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}

static void sip_log(int level, const char *buffer, int len){
    fprintf(stderr, "%.*s", len, buffer);
}

void init_sip(){
    pj_status_t status;

    /* Create pjsua first! */
    pj_log_set_level(0); // no pjsua_create output on stdout please!
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.cb.on_incoming_call = &on_incoming_call;
        cfg.cb.on_call_media_state = &on_call_media_state;
        cfg.cb.on_call_state = &on_call_state;

        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = 3;
        log_cfg.cb = sip_log;

        status = pjsua_init(&cfg, &log_cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
        pjsua_transport_config cfg;

        pjsua_transport_config_default(&cfg);
        cfg.port = 5060;
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);
}

void register_sip(){
    pj_status_t status;
    {
        pjsua_acc_config cfg;

        pjsua_acc_config_default(&cfg);
        cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
        cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
        cfg.cred_count = 1;
        cfg.cred_info[0].realm = pj_str((char*)"*");
        cfg.cred_info[0].scheme = pj_str("digest");
        cfg.cred_info[0].username = pj_str(SIP_USER);
        cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cfg.cred_info[0].data = pj_str(SIP_PASSWD);

        status = pjsua_acc_add(&cfg, PJ_TRUE, &sip_acc_id);
        if (status != PJ_SUCCESS) error_exit("Error adding account", status);
    }
}

void cleanup_sip(){
    pjsua_call_hangup_all();
    pjsua_destroy();
}

void sip_dial(char* dialout_digits){
#ifdef APP_PREVENT_999_CALL
    if(strcmp(dialout_digits, "999") == 0 || strcmp(dialout_digits, "112") == 0 || strcmp(dialout_digits, "911") == 0){
        fprintf(stderr, "WARNING: PREVENTING ACCIDENTAL 999 CALL IN DEVELOPER MODE\n");
        return;
    }
#endif
    if(dialout_digits[0] == '\0') return; // invalid digits
    pj_status_t status;
    char* sip_uri;
    asprintf(&sip_uri, "sip:%s@" SIP_DOMAIN, dialout_digits);
    fprintf(stderr, "Dialling %s...\n", sip_uri);
    if(pjsua_verify_url(sip_uri) != PJ_SUCCESS){
        // invalid call uri
        free(sip_uri);
        return;
    }
    pj_str_t uri = pj_str(sip_uri);
    status = pjsua_call_make_call(sip_acc_id, &uri, 0, NULL, NULL, NULL);
    free(sip_uri);
    if (status != PJ_SUCCESS) error_exit("Error making call", status);
}

int sip_answer(){
    if(!sip_line_active) return EXIT_FAILURE;
    pjsua_call_answer(sip_active_call_id, 200, NULL, NULL);
    return EXIT_SUCCESS;
}

int sip_hangup(){
    if(!sip_line_active) return EXIT_FAILURE;
    pjsua_call_hangup(sip_active_call_id, 603, NULL, NULL);
    return EXIT_SUCCESS;
}

/*
 * main()
 *
 * argv[1] may contain URL to call.
 *//*

int main2(int argc, char *argv[])
{

    */
/* If URL is specified, make call to the URL. *//*

    if (argc > 1) {
        pj_str_t uri = pj_str(argv[1]);
        status = pjsua_call_make_call(sip_acc_id, &uri, 0, NULL, NULL, NULL);
        if (status != PJ_SUCCESS) error_exit("Error making call", status);
    }

    */
/* Wait until user press "q" to quit. *//*

    for (;;) {
        char option[10];

        puts("Press 'h' to hangup all calls, 'q' to quit");
        if (fgets(option, sizeof(option), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (option[0] == 'q')
            break;

        if (option[0] == 'h')
            pjsua_call_hangup_all();
    }

    */
/* Destroy pjsua *//*

    pjsua_destroy();

    return 0;
}*/
