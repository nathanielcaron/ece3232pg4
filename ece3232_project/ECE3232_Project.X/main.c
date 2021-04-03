/*
 * File:   main.c
 * 
 * Group 4:
 * Maxime Boudreau
 * Nathaniel Caron
 * Sam Hache
 * Sahil Saini
 */

// Define FCY for system
#define FCY 16000000UL

// Trigger = MKbus B pin AN = an_2 = RD10
// Echo = MKbus B pin RST = rst_2 = RD11
#define SENSOR_TRIGGER LATDbits.LATD10
#define SENSOR_ECHO PORTDbits.RD11

// Octave LED 1 (Green LED) = MKbus A pin RST = rst_1 = RC7
// Octave LED 2 (Yellow LED) = MKbus A pin CS = cs_1 = RB2
#define OCTAVE_LED_1 LATCbits.LATC7
#define OCTAVE_LED_2 LATBbits.LATB2

// Octave Button 1 (Green Button) = MKbus A pin SCK = sck_1 = RB7
// Octave Button 2 (Yellow Button) = MKbus A pin MISO = miso_1 = RB8
#define OCTAVE_BUTTON_1 PORTBbits.RB7
#define OCTAVE_BUTTON_2 PORTBbits.RB8

// Volume dial = MKbus A pin AN = an_1 = RC0
#define VOLUME_DIAL PORTCbits.RC0

// DS = Data Pin = MKbus B pin INT = int_2 = RB15
// SHCP = Clock pin = MKbus B pin CS = cs_2 = RC3
// STCP = latch pin = MKbus A pin INT = int_1 = RC14
#define SHIFT_REG_DATA LATBbits.LATB15
#define SHIFT_REG_CLOCK LATCbits.LATC3
#define SHIFT_REG_LATCH LATCbits.LATC14

#include "xc.h"
#include "libpic30.h"
#include "math.h"

#pragma config ICS = 2          // set to PGC2/PGD2
#pragma config FNOSC = PRI      // Oscillator Source Selection (Primary Oscillator (XT, HS, EC))
#pragma config POSCMD = HS      // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)

// Global variables
int DisplayValues[9][8];
unsigned long distance = 0;
unsigned long duration = 0;
char NOTE = 'X';
char PREVIOUS_NOTE = 'X';
int OCTAVE = 0;
int VOLUME_DIVIDER = 1;
int NOTE_DURATION = 1;
int sine[] = {
1843,1958,2073,2188,2301,2412,2521,2627,
2730,2830,2925,3017,3104,3186,3262,3333,
3398,3457,3510,3556,3595,3627,3652,3670,
3681,3685,3681,3670,3652,3627,3595,3556,
3510,3457,3398,3333,3262,3186,3104,3017,
2925,2830,2730,2627,2521,2412,2301,2188,
2073,1958,1843,1727,1612,1497,1384,1273,
1164,1058,955,855,760,668,581,499,
423,352,287,228,175,129,90,58,
33,15,4,0,4,15,33,58,
90,129,175,228,287,352,423,499,
581,668,760,855,955,1058,1164,1273,
1384,1497,1612,1727,1843
}; // size 100

// Function templates
void pinSetup();
void setCurrentNote();
unsigned long pulseInHigh();
void seven_segment_setup();
void shiftOut(int values[]);
void WriteUART(char value);
void make_note(void);
void set_timer(void);
void play_melody(void);

int main(void) {
    // Setup all I/O pins
    pinSetup();
    
    // Setup 7-segment display
    seven_segment_setup();

    while(1) {
        
        if ((OCTAVE_BUTTON_1 == 0 && OCTAVE_BUTTON_2 == 1) || (OCTAVE_BUTTON_1 == 1 && OCTAVE_BUTTON_2 == 0)) {
            if (OCTAVE_BUTTON_1 == 0) {
                // Octave 1 selected
                OCTAVE = 1;
                OCTAVE_LED_1 = 1;
                OCTAVE_LED_2 = 0;
            } else if (OCTAVE_BUTTON_2 == 0) {
                // Octave 2 selected
                OCTAVE = 2;
                OCTAVE_LED_1 = 0;
                OCTAVE_LED_2 = 1;
            }

            /* \/\/\/ Get distance reading from sensor \/\/\/ */

            // Clear trigger pin
            SENSOR_TRIGGER = 0;
            __delay_us(2);

            // Produce sound wave
            SENSOR_TRIGGER = 1;
            __delay_us(10);
            SENSOR_TRIGGER = 0;

            // Calculate and output distance in cm
            duration = pulseInHigh();
            distance = ((duration * 0.034) / 2);

            // Distance slope error correction (The sensor reading is increasingly inaccurate)
            distance = distance + (distance*0.1);

            /* /\/\/\ Get distance reading from sensor /\/\/\ */

            setCurrentNote();

            // Visualize notes
            if (NOTE == 'C') {
                // Display C
                shiftOut(DisplayValues[3]);
            } else if (NOTE == 'B') {
                // Display B
                shiftOut(DisplayValues[2]);
            } else if (NOTE == 'A'){
                // Display A
                shiftOut(DisplayValues[1]);
            } else if (NOTE == 'G'){
                // Display G
                shiftOut(DisplayValues[0]);
            } else if (NOTE == 'F'){
                // Display F
                shiftOut(DisplayValues[6]);
            } else if (NOTE == 'E'){
                // Display E
                shiftOut(DisplayValues[5]);
            } else if (NOTE == 'D'){
                // Display D
                shiftOut(DisplayValues[4]);
            } else if (NOTE == 'c') {
                // Display c
                shiftOut(DisplayValues[7]);
            } else {
                // Display Nothing
                shiftOut(DisplayValues[8]);
            }
            
            // Send Note over UART
            if (NOTE != 'X') {
                WriteUART(NOTE);
            }
            WriteUART('\n');
            
            // Do not play note if volume is 0
            if (VOLUME_DIVIDER != 0) {
                // play note on speaker
                make_note();
            }

            PREVIOUS_NOTE = NOTE;
            
            // Reset timer for no user action
            // Trigger Set to 1 - starts timer
            CCP1STATL = 0x40;

        } else {
            // Play nothing
            NOTE = 'X';
            OCTAVE = 0;
            // Display Nothing
            shiftOut(DisplayValues[8]);
            OCTAVE_LED_1 = 0;
            OCTAVE_LED_2 = 0;
        }
    }

    return 0;
}

void pinSetup() {
    /* Setup Proximity sensor pins */
    // set pin RD10 to an output
    TRISDbits.TRISD10 = 0;
    // set pin to digital
    ANSELDbits.ANSELD10 = 0;
    // set pin RD11 to an input
    TRISDbits.TRISD11 = 1;
    // set pin to digital
    ANSELDbits.ANSELD11 = 0;
    
    /* Setup Octave LED pins */
    // set pin rst_1 to an output
    TRISCbits.TRISC7 = 0;
    // set pin to digital
    ANSELCbits.ANSELC7 = 0;
    // set pin cs_1 to an output
    TRISBbits.TRISB2 = 0;
    // set pin to digital
    ANSELBbits.ANSELB2 = 0;
    
    /* Setup Octave Button pins */
    // set pin sck_1 to an input
    TRISBbits.TRISB7 = 1;
    // set pin to digital
    ANSELBbits.ANSELB7 = 0;
    // set pin miso_1 to an input
    TRISBbits.TRISB8 = 1;
    // set pin to digital
    ANSELBbits.ANSELB8 = 0;
    
    /* Setup Volume dial pins */
    // set pin sck_1 to an input
    TRISCbits.TRISC0 = 1;
    // set pin to analog
    ANSELCbits.ANSELC0 = 1;

    /* Setup Shift Register pins */
    // set pin int_2 to an output
    TRISBbits.TRISB15 = 0; 
    // set pin int_2 to an output
    TRISCbits.TRISC3 = 0;
    // set pin int_1 to an output
    TRISCbits.TRISC14 = 0;
    
    // UART Setup (115200 baud rate)
    // Setting the TX_2 to output
    TRISBbits.TRISB13 = 0;
    // Setting TX_2 pin to UART1 Transmit
    RPOR6bits.RP45R = 1;
    // UART Enable bit
    U1MODEbits.UARTEN = 1;
    // Baud Clock Source Selection bits
    U1MODEHbits.BCLKSEL = 0;
    // High Baud Rate Select Bit
    U1MODEbits.BRGH = 1;
    // Setting the baud clock to 115200
    U1BRGbits.BRG = 34;
    // Setting it to legacy mode
    U1MODEHbits.BCLKMOD = 0;
    // UART Mode bits -- Asynchronous 8-bit UART No parity bit
    U1MODEbits.MOD = 0;
    
    // setting registers for speaker
    // set pin RD15 to an output - spk_enable = output
    TRISDbits.TRISD15 = 0;
    // A3 is speaker output
    TRISAbits.TRISA3 = 0;
    // enable speaker
    LATDbits.LATD15 = 1;
    // Enable DAC modules
    DACCTRL1Lbits.DACON = 1;
    //set clock for DAC, 2 for PLL
    DACCTRL1Lbits.CLKSEL = 11;
    //set no division for clock
    DACCTRL1Lbits.CLKDIV = 00;
    // Enable DAC 1
    DAC1CONLbits.DACEN = 1;
    // Enable DAC 1 output buffer
    DAC1CONLbits.DACOEN = 1;
    
    // Setup MCCP timer
    // select the Time Base/Output Compare mode of the module
    CCP1CON1Lbits.CCSEL = 0;
    // 32-bit time base operation
    CCP1CON1Lbits.T32 = 1;
    // timer mode
    CCP1CON1Lbits.MOD = 0;
    // Set the clock source (Tcy)
    CCP1CON1Lbits.CLKSEL = 0;
    // enable timer module
    CCP1CON1Hbits.SYNC = 0;
    // Set Sync/Triggered mode (Triggered Mode), can be triggered by software or hardware
    CCP1CON1Hbits.TRIGEN = 1; 
    // Time base can be retriggered when CCPTRIG = 1, second trigger event occurring during trigger operation will
    // cause the timer to reset and start counting again.
    CCP1CON1Hbits.RTRGEN = 1; 
    // 30 second timer - 48e6/16MHz = 30 seconds
    CCP1PRL = 0x3800; // Low bits
    CCP1PRH = 0x1C9C; // High bits
    // enable timer interrupt
    _CCT1IE = 1;
    // enable timer module
    CCP1CON1Lbits.CCPON = 1;
    // Trigger Set to 1 - starts timer
    CCP1STATL = 0x40;

    //Setup ADC channel for an_1 pin
    //provide max time for initialization
    ADCON5Hbits.WARMTIME = 15; 
    //enable ADC
    ADCON1Lbits.ADON = 1;
    //turning on shared core module power
    ADCON5Lbits.SHRPWR = 1;
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN = 1;
    // set port C0 as input
    TRISCbits.TRISC0 = 1;
    //set port C0 as analog
    ANSELCbits.ANSELC0 = 1;
    //set clock selection 
    ADCON3Hbits.CLKSEL = 0;
    ADCON3Hbits.CLKDIV = 0;
    //clock period (2 clock cycle)
    ADCON2Lbits.SHRADCS = 0;
    //reference select
    ADCON3Lbits.REFSEL = 0;
    //resolution of 12 bits
    ADCON1Hbits.SHRRES = 2;
    //integer output
    ADCON1Hbits.FORM = 0;
    //sample time selection
    ADCON2Hbits.SHRSAMC = 6;
    //ADC interrupts setup
    ADIELbits.IE12 = 1;
    _ADCAN12IF = 0;
    _ADCAN12IE = 1;
    ADCON3Lbits.CNVCHSEL = 12;
    ADCON3Lbits.SHRSAMP = 1;
}

// Convert the distance in cm to a music note (Scale: C, D, E, F, G, A, B, c)
void setCurrentNote() {
    if (distance < 15) {
        NOTE = 'C';
    } else if (distance < 20) {
        NOTE = 'D';
    } else if (distance < 25){
        NOTE = 'E';
    } else if (distance < 30){
        NOTE = 'F';
    } else if (distance < 35){
        NOTE = 'G';
    } else if (distance < 40){
        NOTE = 'A';
    } else if (distance < 45){
        NOTE = 'B';
    } else if (distance < 50){
        NOTE = 'c';
    } else {
        // Invalid note
        NOTE = 'X';
    }
}

unsigned long pulseInHigh() {
	unsigned long echo_signal_width = 0;

	// wait for any previous pulse to end
	while (SENSOR_ECHO == 1) {}

    // wait for the pulse to start
	while (SENSOR_ECHO == 0) {
    }

    // wait for the pulse to stop
	while (SENSOR_ECHO == 1) {
		echo_signal_width++;
	}

    // convert the reading to microseconds.
	return (echo_signal_width * 12 + 15)*(0.0625);
}

// Function to initialize the DisplayValues array and seven segment display
void seven_segment_setup() {
    // Display G (9)
    DisplayValues[0][0] = 1;
    DisplayValues[0][1] = 0;
    DisplayValues[0][2] = 0;
    DisplayValues[0][3] = 1;
    DisplayValues[0][4] = 0;
    DisplayValues[0][5] = 0;
    DisplayValues[0][6] = 0;
    DisplayValues[0][7] = 0;
    
    // Display A
    DisplayValues[1][0] = 1;
    DisplayValues[1][1] = 0;
    DisplayValues[1][2] = 0;
    DisplayValues[1][3] = 0;
    DisplayValues[1][4] = 1;
    DisplayValues[1][5] = 0;
    DisplayValues[1][6] = 0;
    DisplayValues[1][7] = 0;
    
    // Display B
    DisplayValues[2][0] = 1;
    DisplayValues[2][1] = 0;
    DisplayValues[2][2] = 0;
    DisplayValues[2][3] = 0;
    DisplayValues[2][4] = 0;
    DisplayValues[2][5] = 0;
    DisplayValues[2][6] = 1;
    DisplayValues[2][7] = 1;
    
    // Display C
    DisplayValues[3][0] = 1;
    DisplayValues[3][1] = 1;
    DisplayValues[3][2] = 0;
    DisplayValues[3][3] = 0;
    DisplayValues[3][4] = 0;
    DisplayValues[3][5] = 1;
    DisplayValues[3][6] = 1;
    DisplayValues[3][7] = 0;
    
    // Display D
    DisplayValues[4][0] = 1;
    DisplayValues[4][1] = 0;
    DisplayValues[4][2] = 1;
    DisplayValues[4][3] = 0;
    DisplayValues[4][4] = 0;
    DisplayValues[4][5] = 0;
    DisplayValues[4][6] = 0;
    DisplayValues[4][7] = 1;
    
    // Display E
    DisplayValues[5][0] = 1;
    DisplayValues[5][1] = 0;
    DisplayValues[5][2] = 0;
    DisplayValues[5][3] = 0;
    DisplayValues[5][4] = 0;
    DisplayValues[5][5] = 1;
    DisplayValues[5][6] = 1;
    DisplayValues[5][7] = 0;
    
    // Display F
    DisplayValues[6][0] = 1;
    DisplayValues[6][1] = 0;
    DisplayValues[6][2] = 0;
    DisplayValues[6][3] = 0;
    DisplayValues[6][4] = 1;
    DisplayValues[6][5] = 1;
    DisplayValues[6][6] = 1;
    DisplayValues[6][7] = 0;
    
    // Display c (lower case)
    DisplayValues[7][0] = 1;
    DisplayValues[7][1] = 0;
    DisplayValues[7][2] = 1;
    DisplayValues[7][3] = 0;
    DisplayValues[7][4] = 0;
    DisplayValues[7][5] = 1;
    DisplayValues[7][6] = 1;
    DisplayValues[7][7] = 1;
    
    // Display Nothing
    DisplayValues[8][0] = 1;
    DisplayValues[8][1] = 1;
    DisplayValues[8][2] = 1;
    DisplayValues[8][3] = 1;
    DisplayValues[8][4] = 1;
    DisplayValues[8][5] = 1;
    DisplayValues[8][6] = 1;
    DisplayValues[8][7] = 1;
    
    // Display Nothing
    shiftOut(DisplayValues[8]);
}

// Function to shift in new value bits
void shiftOut(int values[]) {
    int i;
    for(i =0; i<8; i++) {
        if(values[i] == 1) {
            SHIFT_REG_DATA = 1;
        } else {
            SHIFT_REG_DATA = 0;
        }
        
        SHIFT_REG_CLOCK = 1;
        __delay_us(500);
        SHIFT_REG_CLOCK = 0;
    }
    SHIFT_REG_LATCH = 1;
    __delay_us(500);
    SHIFT_REG_LATCH = 0;
}

void WriteUART(char value) {
    U1MODEbits.UTXEN = 0; 
    U1TXREGbits.TXREG = value;
    U1MODEbits.UTXEN = 1;
}


/** 
 * Function for playing note to speaker - *FCY must be 16 Mhz*, this function
 * requires the following global variables:
 * int volume - volume control as integer value
 * int duration - time duration in units seconds or inverse seconds. Use seconds
 * for duration > 1 second, inverse seconds for duration < 1 second. More
 * instructions for the duration are found below (by int x instruction)
 * char note - note character to be played. 'C' is lower C note, 'c' is higher c
 * note
 * int octave - octave 1 (lower pitch), octave 2 (higher pitch)
*/
void make_note(void){
    // Do not play note if volume is 0
    if (VOLUME_DIVIDER == 0) return;
    
    // clock cycles for note frequencies for octave 1 - whole notes (C,D,E,F,G,A,B,C)
    // Calculated by: f = 16Mhz/(100*cycles)
    int notes[] = {1224, 1090, 971, 917, 817, 728, 648, 612};
    int cycles;
    // get frequency for note
    if (NOTE == 'C') cycles = notes[0];
    if (NOTE == 'D') cycles = notes[1];
    if (NOTE == 'E') cycles = notes[2];
    if (NOTE == 'F') cycles = notes[3];
    if (NOTE == 'G') cycles = notes[4];
    if (NOTE == 'A') cycles = notes[5];
    if (NOTE == 'B') cycles = notes[6];
    if (NOTE == 'c') cycles = notes[7];
    if (NOTE == 'X') return;
    
    // calculate duration (number of while loop iterations)
    // x is loop iterations to play note for duration, x = duration*note_frequency
    // normally, would say x = duration*16Mhz/(100*cycles), but dsPIC is 16-bit, so
    // cannot do operations on numbers greater than 65535. So broke up into 2 lines.
    int x = 16000/cycles;
    
    // if duration > 1 second, change '/' to '*', if '/' is used, will play note for
    // 1/duration seconds, if '*' used, will play for 'duration' seconds
    x = OCTAVE*x*10/NOTE_DURATION;
    int count = 0; // count iterations
    int i = 0; // count for for-loop (sine values)
    // uncomment this line for ADC channel use
    ADCON3Lbits.CNVRTCH = 1;
    while(count <= x){
        for (i = 0; i<=99; i++){
         DAC1DATHbits.DACDATH = (sine[i]/VOLUME_DIVIDER)+205;
        // argument of delay is cycles, f = 16Mhz/(100*cycles), the {-50} is a
        // correction factor to account for the clock cycles for all instructions
        // in the loop, and is subject to change if any instructions in loop change. 
        // Octave is either 1 or 2, dividing cycles by 2 in delay
        // function doubles the frequency.
         __delay32(cycles/OCTAVE-50); 
        } // end for
        i = 0;
        count++;
    } // end while 1
}

void __attribute__((interrupt, auto_psv)) _CCT1Interrupt(void){
    CCP1STATL = 0x20; // clear trigger
    _CCT1IF = 0; // interrupt flag cleared
    play_melody(); // play note
    CCP1STATL = 0x40; // Trigger Set to 1, start timer again
}

void play_melody(void){
    char temp = NOTE; // saving current note
    int temp2 = OCTAVE; // saving current octave value
    OCTAVE = 2;
    NOTE = 'C';
    make_note();
    NOTE = 'D';
    make_note();
    NOTE = 'E';
    make_note();
    NOTE = 'F';
    make_note();
    NOTE = 'G';
    make_note();
    NOTE = 'A';
    make_note();
    NOTE = 'B';
    make_note();
    NOTE = 'c';
    make_note();
    
    // housekeeping
    NOTE = temp;
    OCTAVE = temp2;
}

void __attribute__((interrupt, no_auto_psv)) _ADCAN12Interrupt(void){
    //interrupt service routine for AN 12 channel
    //interrupt enables must be set
	int potentiometer;
	//get data from appropriate output buffer
    potentiometer = ADCBUF12;
	if(potentiometer < 500) VOLUME_DIVIDER = 1;
	else if(potentiometer > 500 && potentiometer < 3500) VOLUME_DIVIDER = 2;
	else if(potentiometer > 3500 ) VOLUME_DIVIDER = 0;
	
	//reset the flag
    _ADCAN12IF = 0;
}
