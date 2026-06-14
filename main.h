/*
 * File:   main.h
 * Author: luc
 *
 * Created on February 26, 2023, 11:37 AM
 */
#opt 9
#ifndef main_H
#include <16F690.h>
#device ADC=8
#include <math.h>
#include <stdlib.h>
#define main_H
#fuses HS
#fuses NOPROTECT
#fuses BROWNOUT
#fuses NOWDT
#fuses NOMCLR
#case
#byte PIR1 = 0x0C
#byte PIE1 = 0x8C
#bit TMR2IE = PIE1.1
#define AMP 127
//#define AMP_MAX 255
#define ADC_MAX 255
#define SIN_SAMPLES 32
#define SIN8_SAMPLES 8
#define DDS_INDEX_BITS 5
#define DDS_LUT_SIZE (1 << DDS_INDEX_BITS)
#define DDS_PHASE_TO_INDEX_SHIFT (32 - DDS_INDEX_BITS)
#define TIMER2_PRESCALER 4
#define DDS_UPDATE_HZ ((double)MCU_FREQ_MHZ/(TIMER2_PRESCALER*(TIMER2_PERIOD+1)))
#define DDS_PHASE_SCALE 4294967296.0
#define DDS_PHASE_STEP_FROM_HZ(freq_hz) ((unsigned int32)((((freq_hz) * DDS_PHASE_SCALE) / DDS_UPDATE_HZ) + 0.5))
#define TIMER2_PERIOD 63
#define MCU_FREQ_MHZ 2500000
#define PTT_ON 1
#define PTT_OFF 0
#define AMPLITUDE_CHANNEL 10
#define AMPLITUDE_PORT sAN10
#byte OSCCON=0x8F
#bit OSTS=0x8F.3
#bit SCS=0x8F.0
//#byte CONFIG=0x2007

#use delay (clock=10MHz,crystal=10MHz)
#use fast_io(A)
#use fast_io(B)
#use fast_io(C)
#use rs232(uart1,baud=9600)

#define STATE_IDLE       0
#define STATE_TONE_START 1
#define STATE_TONE_ON    2
#define STATE_TONE_TAIL  3
//enum state_enum {
//    idle,
//    tone_start,
//    tone_on,
//    tone_tail,
//};

unsigned long SinAmp[32];
//unsigned long SinAmp8[8];
void updateSinAmpTable(void);
void get_tone_sel(void);
void getAmplitude(void);
void debug(unsigned int line,char* str);

const unsigned int SinTable32[] ={
    127, 152, 176, 198, 217, 233, 244, 252,
    254, 252, 244, 233, 217, 198, 176, 152,
    127, 102, 78, 56, 37, 21, 10, 2,
    0, 2, 10, 21, 37, 56, 78, 102
};

void start_tone(void);
void stop_tone(void);
unsigned int sint( unsigned int& v);
void set_ctcss_period(unsigned int& p);
unsigned int32 phase_accumulator;
unsigned int32 phase_step;
unsigned long tail_counter;
unsigned int amplitude;
unsigned int sin_index = 0;
short rtc_flag = 0;

//unsigned int16 update_dc_count;
unsigned int ctcss_sel;
short ptt_in;
short reverseBurst;
short toneDisable;
short masterEnable;
short CLOCK_FAIL_FLAG;
//int1 RBFlag;
//short ctcss_on;
//#define ENABLE_CTCSS_PIN PIN_B4
short ADC_FLAG;
#define REVERSE_BURST PIN_C7
#define TONE_DISABLE_PIN PIN_A3 
#define PTT_IN PIN_C4
#define PTT_OUT PIN_C3
#define TONE_OUT_PIN PIN_C5
#define MASTER_ENABLE_PIN PIN_B5
// Need between 150 and 200ms
// Decrement occurs at DDS update rate.
#define TAIL_DURATION_MS 150
#define DDS_TAIL_COUNT_MAX ((unsigned long)((((TAIL_DURATION_MS * DDS_UPDATE_HZ) / 1000.0) + 0.5)))
//#define REVERSE_BURST_COUNTER_MAX (150/44)*1000

void initialize(void);

// 32-bit DDS phase increment for each CTCSS frequency at DDS_UPDATE_HZ.
// Calculation: phase_step = round((tone_hz * 2^32) / DDS_UPDATE_HZ).
const unsigned int32 CTCSS_PHASE_STEP[] = {
    DDS_PHASE_STEP_FROM_HZ(67.0),   // 0: 67.0 Hz
    DDS_PHASE_STEP_FROM_HZ(69.3),   // 1: 69.3 Hz
    DDS_PHASE_STEP_FROM_HZ(71.9),   // 2: 71.9 Hz
    DDS_PHASE_STEP_FROM_HZ(74.4),   // 3: 74.4 Hz
    DDS_PHASE_STEP_FROM_HZ(77.0),   // 4: 77.0 Hz
    DDS_PHASE_STEP_FROM_HZ(79.7),   // 5: 79.7 Hz
    DDS_PHASE_STEP_FROM_HZ(82.5),   // 6: 82.5 Hz
    DDS_PHASE_STEP_FROM_HZ(85.4),   // 7: 85.4 Hz
    DDS_PHASE_STEP_FROM_HZ(88.5),   // 8: 88.5 Hz
    DDS_PHASE_STEP_FROM_HZ(91.5),   // 9: 91.5 Hz
    DDS_PHASE_STEP_FROM_HZ(94.8),   // 10: 94.8 Hz
    DDS_PHASE_STEP_FROM_HZ(97.4),   // 11: 97.4 Hz
    DDS_PHASE_STEP_FROM_HZ(100.0),  // 12: 100.0 Hz
    DDS_PHASE_STEP_FROM_HZ(103.5),  // 13: 103.5 Hz
    DDS_PHASE_STEP_FROM_HZ(107.2),  // 14: 107.2 Hz
    DDS_PHASE_STEP_FROM_HZ(110.9),  // 15: 110.9 Hz
    DDS_PHASE_STEP_FROM_HZ(114.8),  // 16: 114.8 Hz
    DDS_PHASE_STEP_FROM_HZ(118.8),  // 17: 118.8 Hz
    DDS_PHASE_STEP_FROM_HZ(123.0),  // 18: 123.0 Hz
    DDS_PHASE_STEP_FROM_HZ(127.3),  // 19: 127.3 Hz
    DDS_PHASE_STEP_FROM_HZ(131.8),  // 20: 131.8 Hz
    DDS_PHASE_STEP_FROM_HZ(136.5),  // 21: 136.5 Hz
    DDS_PHASE_STEP_FROM_HZ(141.3),  // 22: 141.3 Hz
    DDS_PHASE_STEP_FROM_HZ(146.2),  // 23: 146.2 Hz
    DDS_PHASE_STEP_FROM_HZ(151.4),  // 24: 151.4 Hz
    DDS_PHASE_STEP_FROM_HZ(156.7),  // 25: 156.7 Hz
    DDS_PHASE_STEP_FROM_HZ(162.2),  // 26: 162.2 Hz
    DDS_PHASE_STEP_FROM_HZ(167.9),  // 27: 167.9 Hz
    DDS_PHASE_STEP_FROM_HZ(173.8),  // 28: 173.8 Hz
    DDS_PHASE_STEP_FROM_HZ(179.9),  // 29: 179.9 Hz
    DDS_PHASE_STEP_FROM_HZ(186.2),  // 30: 186.2 Hz
    DDS_PHASE_STEP_FROM_HZ(192.8),  // 31: 192.8 Hz
    DDS_PHASE_STEP_FROM_HZ(203.5),  // 32: 203.5 Hz
    DDS_PHASE_STEP_FROM_HZ(206.5),  // 33: 206.5 Hz
    DDS_PHASE_STEP_FROM_HZ(210.7),  // 34: 210.7 Hz
    DDS_PHASE_STEP_FROM_HZ(218.1),  // 35: 218.1 Hz
    DDS_PHASE_STEP_FROM_HZ(225.7),  // 36: 225.7 Hz
    DDS_PHASE_STEP_FROM_HZ(229.1),  // 37: 229.1 Hz
    DDS_PHASE_STEP_FROM_HZ(233.6),  // 38: 233.6 Hz
    DDS_PHASE_STEP_FROM_HZ(241.8),  // 39: 241.8 Hz
    DDS_PHASE_STEP_FROM_HZ(250.3),  // 40: 250.3 Hz
    DDS_PHASE_STEP_FROM_HZ(254.1),  // 41: 254.1 Hz
    DDS_PHASE_STEP_FROM_HZ(1950.0), // 42: 1950.0 Hz
    DDS_PHASE_STEP_FROM_HZ(2175.0)  // 43: 2175.0 Hz
};

//#define CTCSS_SEL_DEBUG 43
//#define AMPLITUDE_DEBUG 511

const unsigned int ctcss_table_size=sizeof(CTCSS_PHASE_STEP)/sizeof(CTCSS_PHASE_STEP[0]);
#endif

