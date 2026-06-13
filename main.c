#include "main.h"

#INT_TIMER2
void timer2_isr(void) {
        if (reverseBurst) {
            phase_accumulator -= phase_step;
        } else {
            phase_accumulator += phase_step;
        }
        sin_index = (phase_accumulator >> DDS_PHASE_TO_INDEX_SHIFT) & 0x1F;
    unsigned long duty_cycle;
    duty_cycle = SinAmp[sin_index];
    set_pwm1_duty(duty_cycle);
    rtc_flag = 1;
    clear_interrupt(INT_TIMER2);
}
#INT_OSC_FAIL
void clock_fail(void) {
    CLOCK_FAIL_FLAG=1;
}

#INT_AD
void read_adc_isr(void) {
    ADC_FLAG=1;
    clear_interrupt(INT_AD);
}

static void updateSinAmpTableProtected(void) {
    int1 timer2_int_was_enabled;
    timer2_int_was_enabled = TMR2IE;
    if (timer2_int_was_enabled) {
        disable_interrupts(INT_TIMER2);
    }
    updateSinAmpTable();
    if (timer2_int_was_enabled) {
        enable_interrupts(INT_TIMER2);
    }
}

void main() {
    initialize();
		get_tone_sel();
	  unsigned adc_timer=0;
    unsigned state=STATE_IDLE;
	  int1 enable_tone;
    while (1) {
        ptt_in      = (input(PTT_IN)==0); // Active low pin
        toneDisable = (input(TONE_DISABLE_PIN)==0); // Active low pin
			  if ( ctcss_sel > 41 ) {
                // COR Polarity is inverted for tones 1950Hz and 2175Hz. Invert PTT input for tone generation logic in this case.
                enable_tone = ~ptt_in;
			  } else {
  				enable_tone = ptt_in;
				}
        // Disconnect TD - Simplify PIC programming.
        //toneDisable = 0;
        switch(state) {
            case STATE_IDLE:
                if(enable_tone) {
                    state=STATE_TONE_START;
                }
                //output_bit(PTT_OUT, PTT_OFF);
                break;
            case STATE_TONE_START:
                reverseBurst = 0;
    			amplitude = read_adc(ADC_START_AND_READ);
	        	updateSinAmpTableProtected();
                start_tone();
                enable_interrupts(INT_TIMER2);
                output_bit(PTT_OUT, PTT_ON);
                state=STATE_TONE_ON;
                break;
            case STATE_TONE_ON:
                if(!enable_tone) {
                    reverseBurst = (input(REVERSE_BURST)==0); // ActiveLow pin
                    if ( !reverseBurst) {
                        stop_tone();
                    }
                    tail_counter = DDS_TAIL_COUNT_MAX;
                    state=STATE_TONE_TAIL;
                }
                break;
            case STATE_TONE_TAIL:
                if (tail_counter==0) {
                    output_bit(PTT_OUT, PTT_OFF);
                    stop_tone();
                    disable_interrupts(INT_TIMER2);
                    state=STATE_IDLE;
                }
                if (enable_tone) {
                    state=STATE_TONE_START;
                }
                break;
        }
        if (rtc_flag) {
            if (tail_counter) tail_counter--;
            rtc_flag=0;
        }
			  if ( adc_timer ) {
					adc_timer--;
				} else {
					adc_timer=0xFF;
                    getAmplitude();
					read_adc(ADC_START_ONLY);
				}
     		if ( ADC_FLAG ) {
					getAmplitude();
					ADC_FLAG=0;
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
        amplitude = new_amplitude;
        updateSinAmpTableProtected();
    }
    // Luc -- Debug
#ifdef AMPLITUDE_DEBUG
    amplitude = AMPLITUDE_DEBUG;
#endif
}

void get_tone_sel (void) {
    unsigned int dip_val;
    dip_val = (~input_c() & 0x07)<<3;
    ctcss_sel = dip_val;
    dip_val = ~input_a()&0x07;
    ctcss_sel += dip_val;
#ifdef CTCSS_SEL_DEBUG
    ctcss_sel = CTCSS_SEL_DEBUG;
#endif
}
void start_tone(void) {
    masterEnable = (input(MASTER_ENABLE_PIN)==0);
	  get_tone_sel();
#ifdef ENABLE_LCD
    char debug_str[20];
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
#endif
    if (ctcss_sel >= ctcss_table_size) {
        ctcss_sel = 12; // set to 100Hz by default
    }
    phase_step = CTCSS_PHASE_STEP[ctcss_sel];
#ifdef ENABLE_LCD
    sprintf(debug_str,"DDSStep=<%Lu>  ",phase_step);
    debug(2,debug_str);
    sprintf(debug_str,"Timer1=<%Lu>  ",t1_val);
    debug(3,debug_str);
#endif
    if ( ! toneDisable ) {
      setup_ccp1(CCP_PWM);
    }
    disable_interrupts(INT_AD);
}
void stop_tone(void) {
    setup_ccp1(CCP_OFF);
    output_bit(TONE_OUT_PIN,0);
    enable_interrupts(INT_AD);
}

void
initialize(void) {
    CLOCK_FAIL_FLAG=0;
    setup_ccp1(CCP_OFF);
    setup_timer_2(T2_DIV_BY_4, TIMER2_PERIOD, 1);
    setup_timer_1(T1_DIV_BY_1 | T1_INTERNAL);
    enable_interrupts(INT_OSC_FAIL);
    enable_interrupts(INT_AD);
    enable_interrupts(GLOBAL);
    set_tris_a(0x2F);
    set_tris_b(0xF0); // Not used
    set_tris_c(0xD7); // Inputs RC[2:0], RC[4,6,7]
    setup_adc(ADC_CLOCK_INTERNAL);
    setup_adc_ports(AMPLITUDE_PORT);
    set_adc_channel(AMPLITUDE_CHANNEL);
    read_adc(ADC_START_ONLY);
    d_val = DDS_TIMER1_TICKS;
    t1_val = 65536UL - (unsigned long)d_val + (unsigned long)TIMER1_LATENCY;
    sin_index = 0;
    phase_accumulator = 0;
    phase_step = CTCSS_PHASE_STEP[12];
    amplitude=255;
    masterEnable=1;
    output_bit(PTT_OUT, PTT_OFF);
}

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
