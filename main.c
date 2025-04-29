#include "main.h"

#INT_TIMER1
void timer1_isr(void) {
    setup_timer_1(T1_DISABLED);
    set_timer1(t1_val);
    setup_timer_1(T1_DIV_BY_1 | T1_INTERNAL);
    rtc_flag = 1;
}
#INT_OSC_FAIL
void clock_fail(void) {
    CLOCK_FAIL_FLAG=1;
}

#INT_AD
void read_adc(void) {
    // Every 44uS
    ADC_FLAG=1;
    //clear_interrupt(INT_AD);
}
//#INT_RB

//void int_rb_isr(void) {

 //   RBFlag = 1;
//}

void
main() {
    initialize();
    unsigned int sin_index=0;
    while (1) {
        ptt_in      = (input(PTT_IN)==0); // Active low pin
        toneDisable = (input(TONE_DISABLE_PIN)==0); // Active low pin
        // Disconnect TD - Simplify PIC programming.
        //toneDisable = 0;
        switch(state) {
            case idle:
                if(ptt_in) {
                    state=tone_start;
                }
                //output_bit(PTT_OUT, PTT_OFF);
                break;
            case tone_start:
                reverseBurst = 0;
                start_tone();
                enable_interrupts(INT_TIMER1);
                output_bit(PTT_OUT, PTT_ON);
                state=tone_on;
                break;
            case tone_on:
                if(!ptt_in) {
                    reverseBurst = (input(REVERSE_BURST)==0); // ActiveLow pin
                    // DEBUG
                    //reverseBurst=1;
                    if ( !reverseBurst) {
                        stop_tone();
                    }
                    tail_counter=2*(TAIL_DURATION_MS * 1000)/(21.1*ctcss_sel+400);
                    state=tone_tail;
                }
                break;
            case tone_tail:
                if (tail_counter==0) {
                    output_bit(PTT_OUT, PTT_OFF);
                    stop_tone();
                    disable_interrupts(INT_TIMER1);
                    state=idle;
                }
                if (ptt_in) {
                    state=tone_start;
                }
                break;
        }
        if (rtc_flag) {
            rtc_flag=0;
            if (reverseBurst) {
                sin_index = (sin_index - increment)&0x1F;
            } else {
                sin_index = (sin_index + increment)&0x1F;
            }
            if ( tail_counter ) {
                tail_counter--;      
            }
            // RESULT OF Y OVERFLOWS!!!
            // Sin varies from 0 to 2*127
            set_ctcss_period(sin_index);
            getAmplitude();
        }

    }
}

void debug(unsigned int line,char* str) {
    putc(line);
    printf(str);
}

void getAmplitude(void) {
    unsigned int new_amplitude;
    new_amplitude = read_adc(ADC_READ_ONLY);
    if ( abs((int)new_amplitude-(int)amplitude) > 3 ) {
        updateSinAmpTable();
        amplitude = new_amplitude;
    }
    read_adc(ADC_START_ONLY);
}

void start_tone(void) {
    unsigned int dip_val;
    masterEnable = (input(MASTER_ENABLE_PIN)==0);
    dip_val = (~input_c() & 0x07)<<3;
    ctcss_sel = dip_val;
    dip_val = ~input_a()&0x07;
    ctcss_sel += dip_val;
    char debug_str[20];

    // Check clock
//    if (setup_oscillator()!=OSC_STATE_STABLE) {
//        sprintf(debug_str,"!CLK");
//        debug(4,debug_str);
//        setup_oscillator(OSC_NORMAL,0);
//    } else {
//        sprintf(debug_str," CLK");
//        debug(4,debug_str);
//    }
    putc(6); // Clear LCD
    putc(4); // Go to line 4.
    if (CLOCK_FAIL_FLAG) {
        CLOCK_FAIL_FLAG=0;
        sprintf(debug_str,":INT!");
        debug(0,debug_str);
    } else {
        sprintf(debug_str,":OK");
        debug(0,debug_str);
    }
    if (OSTS) {
        sprintf(debug_str,"OSTS");
        debug(0,debug_str);
        OSTS=0;
    } else {
        sprintf(debug_str,"!OSTS");
        debug(0,debug_str);
    }
    if (SCS) {
        sprintf(debug_str,"SCS");
        debug(0,debug_str);
    } else {
        sprintf(debug_str,"!SCS");
        debug(0,debug_str);
    }
  
    sprintf(debug_str,"ToneSel=<%d>  ",ctcss_sel);
    debug(1,debug_str);
    if (ctcss_sel > ctcss_table_size) {
        ctcss_sel = 12; // set to 100Hz by default
    }
    if (ctcss_sel < 37) {
        increment = 1;
    } else {
        // Starting at ctcss[37], the MCU is too slow
        // Run the sine wave twice as fast.
        increment = 2;
    }
    //
    // CTCSS tones range from 0-->67Hz to 41-->254.1 Tail must be 150ms.
    //
    // 0.4ms --> 0.123ms
    d_val = CTCSS_T1_FREQ[ctcss_sel];
    t1_val = (2^16) - d_val + (unsigned long)TIMER1_LATENCY;
    
    sprintf(debug_str,"DelayVal=<%Lu>  ",d_val);
    debug(2,debug_str);
    sprintf(debug_str,"Timer1=<%Lu>  ",t1_val);
    debug(3,debug_str);

    if ( ! toneDisable ) {
      setup_ccp1(CCP_PWM);
    }
}
void stop_tone(void) {
    setup_ccp1(CCP_OFF);
    output_bit(TONE_OUT_PIN,0);
}

void
initialize(void) {
    CLOCK_FAIL_FLAG=0;
    setup_ccp1(CCP_OFF);
    setup_timer_2(T2_DIV_BY_4, 255, 1);
    setup_timer_1(T1_DIV_BY_1 | T1_INTERNAL);
    enable_interrupts(INT_OSC_FAIL);
    enable_interrupts(GLOBAL);
    set_tris_a(0x2F);
    set_tris_b(0xF0); // Not used
    set_tris_c(0xD7); // Inputs RC[2:0], RC[4,6,7]
    setup_adc(ADC_CLOCK_INTERNAL);
    setup_adc_ports(AMPLITUDE_PORT);
    set_adc_channel(AMPLITUDE_CHANNEL);
    read_adc(ADC_START_ONLY);
    state=idle;
    amplitude=255;
    masterEnable=1;
    output_bit(PTT_OUT, PTT_OFF);
    char tone_str[20];
    sprintf(tone_str"Hello!");
    debug(4,tone_str);
}

// Second time inside set_ctcss_period
// Substracted once inside sint() below
unsigned int sint(unsigned int& v) {
    // PSAMPLES = 32
    unsigned int angle = v & 0x1F;
    unsigned int index = angle & 0x0F;
    if ((angle & 0x10)) {
        return (AMP - SinTable16[index]);
    } else {
        return (AMP + SinTable16[index]);
    }
}
void updateSinAmpTable(void) {
    int x;
    // TIMER2_PERIOD = 255
    // Sin(t) ranges from 0 to 2*AMP(0:254)
    // Amplitude ranges from 0 to ADC_MAX (0:255)
    // DutyCycle must range from 0% (0) to 100% (4*(TIMER2_PERIOD+1))
    unsigned long duty_cycle;
    for(x=0;x<32;x++) {
        // = 4 * (256)/ (2*128) * (Sin(t) * amp/AMP_MAX)
        // = 4 * 
        duty_cycle = (unsigned long)(4*(TIMER2_PERIOD+1)/(2*(AMP+1))*((unsigned long)sint(x)*amplitude/(ADC_MAX+1)));
        SinAmp[x] = duty_cycle;
    }
}
void set_ctcss_period(unsigned int& index) {
    // p is CTCSS period
    unsigned long duty_cycle;
    duty_cycle=SinAmp[index];
    set_pwm1_duty(duty_cycle);
}


