#include "main.h"
#include <math.h>

unsigned int sin_index = 0;
int1 rtc_flag = 0;
#INT_TIMER0

void
rtcc_isr(void) {
    sin_index++;
    rtc_flag = 1;
}

#INT_TIMER1

void
timer1_isr(void) {
    ctcss_sel = input_a();
    if (ctcss_sel > sizeof (CTCSS_T1_FREQ)) {
        ctcss_sel = 12; // set to 100Hz by default
    }
    unsigned int16 d_val = CTCSS_T1_FREQ[ctcss_sel];
    unsigned int16 t1_val = (2^16 - 1) - d_val;
    set_timer1(t1_val);
    sin_index++;
    rtc_flag = 1;
}

#INT_RB
void int_rb_isr(void) {
    enableCTCSS = input(ENABLE_CTCSS_PIN);
    reverseBurst = input(REVERSE_BURST_PIN);
    RBFlag=1;
}
void
main() {
    unsigned int8 x;
    enableCTCSS=0;
    reverseBurst=0;
    RBFlag=1;
    initialize();
    x = 0;
    //    while(1) {
    //        y = sint(x);
    //        x=(x+1)&0x1F;
    //    }
    while (1) {
        if (RBFlag) {
            if (enableCTCSS) {
                setup_ccp1(CCP_PWM);
                enable_interrupts(INT_TIMER1);
            } else {
                setup_ccp1(CCP_OFF);
                disable_interrupts(INT_TIMER1);
            }
            RBFlag = 0;
        }
        if (rtc_flag) {
            if (reverseBurst) {
                x = (x - 1)&0x1F;
            } else {
                x = (x + 1)&0x1F;
            }
            // RESULT OF Y OVERFLOWS!!!
            // Sin varies from 0 to 2*127
            set_ctcss_period(x);
            rtc_flag = 0;
        }
    }
}
// CTCSS frequencies min/max are 67Hz/257Hz
// At 4MHz FOSC, using a sine wave with 32 samples, 
// The PWM counter values are
// 466 and 122 respectively.
// At 10MHz FOCS, using a sine wave with 16 samples
// The PWM duty cycle must be refreshed 
// between 614 and 2332 cycles.
//

void
initialize(void) {
    setup_ccp1(CCP_OFF);
    setup_timer_2(T2_DIV_BY_4, 255, 1);
    //setup_timer_0(RTCC_INTERNAL);
    //enable_interrupts(INT_TIMER0);
    setup_timer_1(T1_DIV_BY_4 | T1_INTERNAL);
    enable_interrupts(INT_RB4|INT_RB5);
    enable_interrupts(INT_TIMER1);
    enable_interrupts(GLOBAL);
    set_tris_a(0x3F);
    set_tris_b(0xF0);
    

}

unsigned int8 
sint(unsigned int8& v) {
    // PSAMPLES = 32
    unsigned int8 angle = v & 0x1F;
    unsigned int8 index = angle & 0x0F;
    if ((angle & 0x10)) {
        return (AMP - SinTable16[index]);
    } else {
        return (AMP + SinTable16[index]);
    }
}

void
set_ctcss_period(unsigned int& p) {
    // p is CTCSS period
    unsigned int8 y;
    y = (TIMER2_PERIOD / 2 * AMP) * sint(p);
    set_pwm1_duty(y);
}


