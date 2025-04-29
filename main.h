/*
 * File:   main.h
 * Author: luc
 *
 * Created on February 26, 2023, 11:37 AM
 */
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
#define AMP 127
//#define AMP_MAX 255
#define ADC_MAX 255
#define SIN_SAMPLES 32
#define SIN16_SAMPLES 16
#define TIMER2_PERIOD 255
#define MCU_FREQ_MHZ 2500000
#define T1_PRESCALER 1
#define PTT_ON 1
#define PTT_OFF 0
// Timer1 latency consumes 4 instructions from 
// T1_disable to T1 re-enabled.
// Latency=28 is trimmed to work for 100Hz tone.
#define TIMER1_LATENCY 28
#define AMPLITUDE_CHANNEL 10
#define AMPLITUDE_PORT sAN10
#byte OSCCON=0x8F
#bit OSTS=0x8F.3
#bit SCS=0x8F.0
#byte CONFIG=0x2007

#use delay (clock=10MHz,crystal=10MHz)
#use fast_io(A)
#use fast_io(B)
#use fast_io(C)
#use rs232(uart1,baud=9600)

enum state_enum {
    idle,
    tone_start,
    tone_on,
    tone_tail,
} state;

unsigned long SinAmp[32];
void updateSinAmpTable(void);
void getAmplitude(void);
void debug(unsigned int line,char* str);

const unsigned int SinTable16[] ={ 
    AMP * 0,
    AMP * 0.2,
    AMP * 0.38,
    AMP * 0.56,
    AMP * 0.71,
    AMP * 0.83,
    AMP * 0.92,
    AMP * 0.98,
    AMP * 1.0,
    AMP * 0.98,
    AMP * 0.92,
    AMP * 0.83,
    AMP * 0.71,
    AMP * 0.56,
    AMP * 0.38,
    AMP * 0.20};

void start_tone(void);
void stop_tone(void);
unsigned int sint( unsigned int& v);
void set_ctcss_period(unsigned int& p);
unsigned long d_val;
unsigned long t1_val;
unsigned int increment;
unsigned long tail_counter;
unsigned int amplitude;
//unsigned int sin_index = 0;
short rtc_flag = 0;

//unsigned int16 update_dc_count;
unsigned int ctcss_sel;
short ptt_in;
short reverseBurst;
short toneDisable;
short masterEnable;
short CLOCK_FAIL_FLAG;
//int1 RBFlag;
short ctcss_on;
//#define ENABLE_CTCSS_PIN PIN_B4
short ADC_FLAG;
#define REVERSE_BURST PIN_C7
#define TONE_DISABLE_PIN PIN_A3 
#define PTT_IN PIN_C4
#define PTT_OUT PIN_C3
#define TONE_OUT_PIN PIN_C5
#define MASTER_ENABLE_PIN PIN_B5
// Need between 150 and 200ms
// Decrement occurs every 44us
#define TAIL_DURATION_MS 150
//#define REVERSE_BURST_COUNTER_MAX (150/44)*1000

void initialize(void);

// Timer1 values for each CTCSS frequency when running at
// FOSC = 10MHz

const unsigned long CTCSS_T1_FREQ[] = {		// RC[2:0]:RA[2:0]
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/67,    	// 0 (1166)
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/69.3,	 	// 1
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/71.9,		// 2
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/74.4,		// 3
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/77,		// 4
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/79.7,		// 5
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/82.5,		// 6
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/85.4,		// 7
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/88.5,		// 8
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/91.5,		// 9
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/94.8,		// 10
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/97.4,		// 11
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/100,		// 12
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/103.5,	// 13
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/107.2,	// 14
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/110.9,	// 15
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/114.8,	// 16
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/118.8,	// 17
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/123,		// 18
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/127.3,	// 19
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/131.8,	// 20
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/136.5,	// 21
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/141.3,	// 22
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/146.2,	// 23
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/151.4,	// 24
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/156.7,	// 25
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/162.2,	// 26
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/167.9,	// 27
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/173.8,	// 28
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/179.9,	// 29
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/186.2,	// 30
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/192.8,	// 31
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/203.5,	// 32 (384)
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/206.5,	// 33
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/210.7,	// 34
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/218.1,	// 35
    MCU_FREQ_MHZ/SIN_SAMPLES/T1_PRESCALER/225.7,	// 36 (346))
    MCU_FREQ_MHZ/SIN16_SAMPLES/T1_PRESCALER/229.1,	// 37
    MCU_FREQ_MHZ/SIN16_SAMPLES/T1_PRESCALER/233.6,	// 38
    MCU_FREQ_MHZ/SIN16_SAMPLES/T1_PRESCALER/241.8,	// 39
    MCU_FREQ_MHZ/SIN16_SAMPLES/T1_PRESCALER/250.3,	// 40
    MCU_FREQ_MHZ/SIN16_SAMPLES/T1_PRESCALER/254.1,	// 41 (307.5)
};

// Counter Delay Equation:
// 4.8  (67.794 + 1.9447 ctcss_sel + 0.0657 )
// Note that starting with tone 37, the delay must be divided by 2.
const unsigned long tailCounterMax[] = {325, 335, 345, 356, 367, 379, \
392, 406, 420, 434, 450, 466, 482, \
500, 517, 536, 555, 575, 595, 616, 638, 660, 683, 706, 731, 755, 781, \
807, 834, 861, 889, 917, 947, 976, 1007, 1038, \
1070, 551, 568, 585, 602, 619};
//1070, 1102, 1135, 1169, 1203, 1238};

const unsigned int ctcss_table_size=sizeof(CTCSS_T1_FREQ)/2;
#endif

