#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <termios.h>
#include <string.h>

#include "dtmf.h"
#include "sip.h"
#include "ringtone.h"

#define APP_DISPLAY_USE_LCD_DEVICE
#define APP_PREVENT_999_CALL
#define USE_SCROLL_LOCK_AS_HOOK_SWITCH
//#define APP_DISPLAY_USE_STDOUT
#define APP_USE_GPIO


#include "gpio.h"
#include "input.h"
#include "display.h"

#define MAX_DIALOUT_DIGITS  32
uint16_t tone_key_down = KEY_RESERVED;
char dialout_digits[MAX_DIALOUT_DIGITS+1];
size_t count_dialout_digits = 0;
int on_hook = 1;

#ifdef APP_DISPLAY_USE_LCD_DEVICE
FILE* LCD_DEVICE;
#endif /* APP_DISPLAY_USE_LCD_DEVICE */

void callback_on_hook(){
    sip_hangup();
    dtmf_stop_bg();
    dtmf_drain();
    memset(dialout_digits, '\0', sizeof(dialout_digits));
    count_dialout_digits = 0;
    DISPLAY_CLEAR();
    on_hook = 1;
}

void callback_off_hook(){
    dtmf(DTMF_UK_DIALTONE_F1, DTMF_UK_DIALTONE_F2, -1);
    on_hook = 0;
}

void event_callback(struct input_event ie){
    if(ie.type == EV_KEY){
        if(ie.value == 1) {
            char c = keycode_to_printable_char(ie.code);
            if (c != '\0'){
                // while not on call
                if(!on_hook) {
                    uint16_t freq1, freq2;
                    if (count_dialout_digits < MAX_DIALOUT_DIGITS &&
                        char_to_dtmf_freqs(c, &freq1, &freq2) == EXIT_SUCCESS) {
                        if (!sip_is_line_active()) {
                            DISPLAY_PRINTF("%c", c);
                            dialout_digits[count_dialout_digits++] = c;
                        } else {
                            sip_send_dtmf(c);
                        }
                        dtmf(freq1, freq2, -1);
                        tone_key_down = ie.code;
                    } else if (c == '\n') {
                        if (!sip_is_line_active()) {
                            sip_dial(dialout_digits);
                            memset(dialout_digits, '\0', sizeof(dialout_digits));
                            count_dialout_digits = 0;
                            DISPLAY_CLEAR();
                        }
                    } else if (c == '\b') {
                        if (!sip_is_line_active()) {
                            if (count_dialout_digits > 0) {
                                dialout_digits[--count_dialout_digits] = '\0';
                                DISPLAY_PRINT("\b \b");
                            }
                        }
                    }
                }
            }else{
                switch(ie.code){
                    case KEY_PICKUP_PHONE:
                    case KEY_F9:
                        sip_answer();
                        break;
                    case KEY_HANGUP_PHONE:
                    case KEY_F10:
                        sip_hangup();
                        break;
#ifdef USE_SCROLL_LOCK_AS_HOOK_SWITCH
                    case KEY_SCROLLLOCK:
                        callback_off_hook();
                        break;
#endif
                    default:
                        fprintf(stderr,"Keypress code %d\n", ie.code);
                        break;
                }
            }
        }else if(ie.value == 0) {
            if(ie.code == tone_key_down){
                dtmf_stop_bg();
                dtmf_drain();
            }
            switch(ie.code){
#ifdef USE_SCROLL_LOCK_AS_HOOK_SWITCH
                case KEY_SCROLLLOCK:
                    callback_on_hook();
                    break;
#endif
                default:
                    fprintf(stderr,"Keyrelease code %d\n", ie.code);
                    break;
            }
        }
    }else if(ie.type == EV_SW){
        switch(ie.code){
            case APP_SWITCH_HOOK:
                if(ie.value){
                    gpiod_line_set_value(handset_mic_request_line, 1);
                    while(!gpiod_line_get_value(handset_mic_active_line)){
                        fprintf(stderr, "Waiting for mic activation...\n");
                    }
                    fprintf(stderr, "Mic active!\n");
                    callback_off_hook();
                }else{
                    gpiod_line_set_value(handset_mic_request_line, 0);
                    callback_on_hook();
                }
                break;
            case APP_SWITCH_MIC_MUTE:
                break;
        }
    }
}

volatile sig_atomic_t exit_interrupt = 0;
void sigint_callback(int sig){
    exit_interrupt = 1;
}

int main() {
    signal(SIGINT, sigint_callback);
#ifdef APP_DISPLAY_USE_STDOUT
    // get current terminal attributes
    struct termios term_old;
    tcgetattr(fileno(stdin), &term_old);
    // disable terminal echo and output buffer, we want to handle input ourselves
    struct termios term_new = term_old;
    term_new.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(fileno(stdin), 0, &term_new);
    setbuf(stdout, NULL);
#endif /* APP_DISPLAY_USE_STDOUT */
#ifdef APP_DISPLAY_USE_LCD_DEVICE
    LCD_DEVICE = fopen("/dev/lcd", "w");
    if(LCD_DEVICE == NULL){
        perror("Could not open LCD");
        return EXIT_FAILURE;
    }

    DISPLAY_REINIT();
    DISPLAY_ON();
    DISPLAY_BACKLIGHT_ON();
#endif /* APP_DISPLAY_USE_LCD_DEVICE */

    init_dtmf();
    init_input_devices(event_callback);
    init_sip();
    register_sip();
    init_ringtone();
#ifdef APP_USE_GPIO
    init_gpio();
#endif /* APP_USE_GPIO */

    memset(dialout_digits, '\0', sizeof(dialout_digits));

    DISPLAY_CLEAR();
    DISPLAY_CURSOR_BLINK_OFF();
    DISPLAY_PRINT("Welcome to IP Phone");
    DISPLAY_GOTO_XY("5", "2");
    //ringtone_on();

    while(!exit_interrupt){
        do_input_poll(-1);
    }

    // cleanup input devices (close fds)
    fprintf(stderr, "Goodbye!\n");
#ifdef APP_USE_GPIO
    cleanup_gpio();
#endif /* APP_USE_GPIO */
    cleanup_input_devices();
    cleanup_dtmf();
    cleanup_sip();
    cleanup_ringtone();
#ifdef APP_DISPLAY_USE_STDOUT
    // reset terminal attributes
    tcsetattr(fileno(stdin), 0, &term_old);
#endif /* APP_DISPLAY_USE_STDOUT */
#ifdef APP_DISPLAY_USE_LCD_DEVICE
    fclose(LCD_DEVICE);
#endif /* APP_DISPLAY_USE_LCD_DEVICE */
    return 0;
}
