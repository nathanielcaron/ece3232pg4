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
int DisplayValues[17][8];
long duration = 0;
int distance = 0;

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
        // Do something to visualize distance
        if (distance < 10) {
            shiftOut(DisplayValues[0]);
        } else if (distance < 20) {
            shiftOut(DisplayValues[1]);
        } else {
            shiftOut(DisplayValues[2]);
        }
    }

    return 0;
}

unsigned long pulseInHigh() {
	unsigned long echo_signal_width = 0; // keep initialization out of time critical area

	// wait for any previous pulse to end
	while (SENSOR_ECHO == 1) {}

    // wait for the pulse to start
	while (SENSOR_ECHO == 0) {
        SHARP_LED = 1;
    }

    // wait for the pulse to stop
	while (SENSOR_ECHO == 1) {
        SHARP_LED = 0;
		echo_signal_width++;
	}

    // convert the reading to microseconds. The loop has been determined
	// to be 20 clock cycles long and have about 16 clocks between the edge
	// and the start of the loop. There will be some error introduced by
	// the interrupt handlers.
	return (echo_signal_width * 21 + 16)*(0.0625);
}

void seven_segment_setup() {
    // Display 0
    DisplayValues[0][0] = 1;
    DisplayValues[0][1] = 1;
    DisplayValues[0][2] = 0;
    DisplayValues[0][3] = 0;
    DisplayValues[0][4] = 0;
    DisplayValues[0][5] = 0;
    DisplayValues[0][6] = 0;
    DisplayValues[0][7] = 0;
        
    // Display 1
    DisplayValues[1][0] = 1;
    DisplayValues[1][1] = 1;
    DisplayValues[1][2] = 1;
    DisplayValues[1][3] = 1;
    DisplayValues[1][4] = 1;
    DisplayValues[1][5] = 0;
    DisplayValues[1][6] = 0;
    DisplayValues[1][7] = 1;
    
    // Display 2
    DisplayValues[2][0] = 1;
    DisplayValues[2][1] = 0;
    DisplayValues[2][2] = 1;
    DisplayValues[2][3] = 0;
    DisplayValues[2][4] = 0;
    DisplayValues[2][5] = 1;
    DisplayValues[2][6] = 0;
    DisplayValues[2][7] = 0;
    
    // Display 3
    DisplayValues[3][0] = 1;
    DisplayValues[3][1] = 0;
    DisplayValues[3][2] = 1;
    DisplayValues[3][3] = 1;
    DisplayValues[3][4] = 0;
    DisplayValues[3][5] = 0;
    DisplayValues[3][6] = 0;
    DisplayValues[3][7] = 0;
    
    // Display 4
    DisplayValues[4][0] = 1;
    DisplayValues[4][1] = 0;
    DisplayValues[4][2] = 0;
    DisplayValues[4][3] = 1;
    DisplayValues[4][4] = 1;
    DisplayValues[4][5] = 0;
    DisplayValues[4][6] = 0;
    DisplayValues[4][7] = 1;
    
    // Display 5
    DisplayValues[5][0] = 1;
    DisplayValues[5][1] = 0;
    DisplayValues[5][2] = 0;
    DisplayValues[5][3] = 1;
    DisplayValues[5][4] = 0;
    DisplayValues[5][5] = 0;
    DisplayValues[5][6] = 1;
    DisplayValues[5][7] = 0;
    
    // Display 6
    DisplayValues[6][0] = 1;
    DisplayValues[6][1] = 0;
    DisplayValues[6][2] = 0;
    DisplayValues[6][3] = 0;
    DisplayValues[6][4] = 0;
    DisplayValues[6][5] = 0;
    DisplayValues[6][6] = 1;
    DisplayValues[6][7] = 0;
    
    // Display 7
    DisplayValues[7][0] = 1;
    DisplayValues[7][1] = 1;
    DisplayValues[7][2] = 1;
    DisplayValues[7][3] = 1;
    DisplayValues[7][4] = 1;
    DisplayValues[7][5] = 0;
    DisplayValues[7][6] = 0;
    DisplayValues[7][7] = 0;
    
    // Display 8
    DisplayValues[8][0] = 1;
    DisplayValues[8][1] = 0;
    DisplayValues[8][2] = 0;
    DisplayValues[8][3] = 0;
    DisplayValues[8][4] = 0;
    DisplayValues[8][5] = 0;
    DisplayValues[8][6] = 0;
    DisplayValues[8][7] = 0;
    
    // Display 9
    DisplayValues[9][0] = 1;
    DisplayValues[9][1] = 0;
    DisplayValues[9][2] = 0;
    DisplayValues[9][3] = 1;
    DisplayValues[9][4] = 0;
    DisplayValues[9][5] = 0;
    DisplayValues[9][6] = 0;
    DisplayValues[9][7] = 0;
    
    // Display A
    DisplayValues[10][0] = 1;
    DisplayValues[10][1] = 0;
    DisplayValues[10][2] = 0;
    DisplayValues[10][3] = 0;
    DisplayValues[10][4] = 1;
    DisplayValues[10][5] = 0;
    DisplayValues[10][6] = 0;
    DisplayValues[10][7] = 0;
    
    // Display B
    DisplayValues[11][0] = 1;
    DisplayValues[11][1] = 0;
    DisplayValues[11][2] = 0;
    DisplayValues[11][3] = 0;
    DisplayValues[11][4] = 0;
    DisplayValues[11][5] = 0;
    DisplayValues[11][6] = 1;
    DisplayValues[11][7] = 1;
    
    // Display C
    DisplayValues[12][0] = 1;
    DisplayValues[12][1] = 1;
    DisplayValues[12][2] = 0;
    DisplayValues[12][3] = 0;
    DisplayValues[12][4] = 0;
    DisplayValues[12][5] = 1;
    DisplayValues[12][6] = 1;
    DisplayValues[12][7] = 0;
    
    // Display D
    DisplayValues[13][0] = 1;
    DisplayValues[13][1] = 0;
    DisplayValues[13][2] = 1;
    DisplayValues[13][3] = 0;
    DisplayValues[13][4] = 0;
    DisplayValues[13][5] = 0;
    DisplayValues[13][6] = 0;
    DisplayValues[13][7] = 1;
    
    // Display E
    DisplayValues[14][0] = 1;
    DisplayValues[14][1] = 0;
    DisplayValues[14][2] = 0;
    DisplayValues[14][3] = 0;
    DisplayValues[14][4] = 0;
    DisplayValues[14][5] = 1;
    DisplayValues[14][6] = 1;
    DisplayValues[14][7] = 0;
    
    // Display F
    DisplayValues[15][0] = 1;
    DisplayValues[15][1] = 0;
    DisplayValues[15][2] = 0;
    DisplayValues[15][3] = 0;
    DisplayValues[15][4] = 1;
    DisplayValues[15][5] = 1;
    DisplayValues[15][6] = 1;
    DisplayValues[15][7] = 0;
    
    // Display nothing
    DisplayValues[16][0] = 1;
    DisplayValues[16][1] = 1;
    DisplayValues[16][2] = 1;
    DisplayValues[16][3] = 1;
    DisplayValues[16][4] = 1;
    DisplayValues[16][5] = 1;
    DisplayValues[16][6] = 1;
    DisplayValues[16][7] = 1;
    
    shiftOut(DisplayValues[16]);
}

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