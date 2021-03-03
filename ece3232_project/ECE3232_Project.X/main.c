/*
 * File:   main.c
 * Author: Nathaniel Caron
 *
 * Created on February 27, 2021, 4:53 PM
 */

// Define constants for common I/O
#define FCY 16000000UL

#define SENSOR_TRIGGER LATDbits.LATD10
#define SENSOR_ECHO PORTDbits.RD11

#define SHARP_LED LATBbits.LATB2

#define SHIFT_REG_DATA LATBbits.LATB15
#define SHIFT_REG_CLOCK LATCbits.LATC3
#define SHIFT_REG_LATCH LATCbits.LATC14


#include "xc.h"
#include "libpic30.h"

#pragma config ICS = 2                  // set to PGC2/PGD2
#pragma config FNOSC = PRI              // Oscillator Source Selection (Primary Oscillator (XT, HS, EC))
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)

// Global variables
int DisplayValues[8][8];
long duration = 0;
int distance = 0;
int previous_distance = -1;

// Function declarations
unsigned long pulseInHigh();
void seven_segment_setup();
void shiftOut(int values[]);

int main(void) {
    /* Proximity sensor setup */
    
    // Trigger = MKbus B pin AN = an_2 = RD10
    // Echo = MKbus B pin RST = rst_2 = RD11
    
    // LED1 = MKbus B pin CS = cs_2 = RC3
    // LED2 = MKbus B pin SCK = sck_2 = RC6
    // LED3 = MKbus B pin MISO = miso_2 = RC2

    // set pin RD10 to an output
    TRISDbits.TRISD10 = 0;
    // set pin to digital
    ANSELDbits.ANSELD10 = 0;
    // set pin RD11 to an input
    TRISDbits.TRISD11 = 1;
    // set pin to digital
    ANSELDbits.ANSELD11 = 0;

    // set pin RC3 to an output
    TRISCbits.TRISC3 = 0;
    // set pin to digital
    ANSELCbits.ANSELC3 = 0;
    // set pin RC6 to an output
    TRISCbits.TRISC6 = 0;
    // set pin to digital
    ANSELCbits.ANSELC6 = 0;
    // set pin RC2 to an output
    TRISCbits.TRISC2 = 0;
    // set pin to digital
    ANSELCbits.ANSELC2 = 0;
    
    /* Proximity sensor setup */

    // DS = Data Pin = MKbus B pin INT = int_2 = RB15
    // SHCP = Clock pin = MKbus B pin CS = cs_2 = RC3
    // STCP = latch pin = MKbus A pin INT = int_1 = RC14
    // Sharp LED = MKbus A pin CS = cs_1 = RB2

    // set pin int_2 to an output
    TRISBbits.TRISB15 = 0; 
    // set pin int_2 to an output
    TRISCbits.TRISC3 = 0;
    // set pin int_1 to an output
    TRISCbits.TRISC14 = 0;
    // set pin cs_1 to an output
    TRISBbits.TRISB2 = 0;
    
    seven_segment_setup();

//    int i;
//    for(i=0; i<16; i++) {
//        shiftOut(DisplayValues[i]);
//        // turn on LED
//        LATBbits.LATB2 = 1;
//        __delay_ms(500);
//        // turn off LED
//        LATBbits.LATB2 = 0;
//        __delay_ms(500);
//    }

    while(1) {
        // Clear trigger pin
        SENSOR_TRIGGER = 0;
        __delay_us(2);
    
        // Produce sound wave
        SENSOR_TRIGGER = 1;
        __delay_us(10);
        SENSOR_TRIGGER = 0;
        
        // Calculate and output distance in cm
        duration = pulseInHigh();
        distance = duration * 0.034 / 2;
        
        // Change note only if different
        if (distance != previous_distance) {
            // Visualize notes
            if (distance >= 10 && distance < 15) {
                // Display C
                shiftOut(DisplayValues[3]);
                SHARP_LED = 0;
            } else if (distance < 20) {
                // Display B
                shiftOut(DisplayValues[2]);
                SHARP_LED = 0;
            } else if (distance < 25){
                // Display A#
                shiftOut(DisplayValues[1]);
                SHARP_LED = 1;
            } else if (distance < 30){
                // Display A
                shiftOut(DisplayValues[1]);
                SHARP_LED = 0;
            } else if (distance < 35){
                // Display G#
                shiftOut(DisplayValues[0]);
                SHARP_LED = 1;
            } else if (distance < 40){
                // Display G
                shiftOut(DisplayValues[0]);
                SHARP_LED = 0;
            } else if (distance < 45){
                // Display F#
                shiftOut(DisplayValues[6]);
                SHARP_LED = 1;
            } else if (distance < 50){
                // Display F
                shiftOut(DisplayValues[6]);
                SHARP_LED = 0;
            } else if (distance < 55){
                // Display E
                shiftOut(DisplayValues[5]);
                SHARP_LED = 0;
            } else if (distance < 60){
                // Display D#
                shiftOut(DisplayValues[4]);
                SHARP_LED = 1;
            } else if (distance < 65){
                // Display D
                shiftOut(DisplayValues[4]);
                SHARP_LED = 0;
            } else if (distance < 70){
                // Display C#
                shiftOut(DisplayValues[3]);
                SHARP_LED = 1;
            } else if (distance < 75){
                // Display C
                shiftOut(DisplayValues[3]);
                SHARP_LED = 0;
            } else {
                // Display Nothing
                shiftOut(DisplayValues[7]);
                SHARP_LED = 0;
            }
            previous_distance = distance;
        }

        __delay_ms(250);
    }

    return 0;
}

unsigned long pulseInHigh() {
	unsigned long echo_signal_width = 0; // keep initialization out of time critical area

	// wait for any previous pulse to end
	while (SENSOR_ECHO == 1) {}

    // wait for the pulse to start
	while (SENSOR_ECHO == 0) {
    }

    // wait for the pulse to stop
	while (SENSOR_ECHO == 1) {
		echo_signal_width++;
	}

    // convert the reading to microseconds. The loop has been determined
	// to be 20 clock cycles long and have about 16 clocks between the edge
	// and the start of the loop. There will be some error introduced by
	// the interrupt handlers.
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
    
    // Display Nothing
    DisplayValues[7][0] = 1;
    DisplayValues[7][1] = 1;
    DisplayValues[7][2] = 1;
    DisplayValues[7][3] = 1;
    DisplayValues[7][4] = 1;
    DisplayValues[7][5] = 1;
    DisplayValues[7][6] = 1;
    DisplayValues[7][7] = 1;
    
    shiftOut(DisplayValues[16]);
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