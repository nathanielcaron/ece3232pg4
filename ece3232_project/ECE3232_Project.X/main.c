/*
 * File:   main.c
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

#pragma config ICS = 2          // set to PGC2/PGD2
#pragma config FNOSC = PRI      // Oscillator Source Selection (Primary Oscillator (XT, HS, EC))
#pragma config POSCMD = HS      // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)

// Global variables
int DisplayValues[8][8];
unsigned long distance = 0;
unsigned long duration = 0;
char NOTE = 'X';
char PREVIOUS_NOTE = 'X';
int octave = 0;

// Function declarations
void pinSetup();
void setCurrentNote();
unsigned long pulseInHigh();
void seven_segment_setup();
void shiftOut(int values[]);
void WriteUART(char value);

int main(void) {
    // Setup all I/O pins
    pinSetup();
    
    // Setup 7-segment display
    seven_segment_setup();
    
    /** TESTS */
    
    OCTAVE_LED_1 = 1;
    OCTAVE_LED_2 = 1;

    int i;
    for (i = 0; i < 8; i++) {
        shiftOut(DisplayValues[i]);
        __delay_ms(500);
    }
    
    /** TESTS */

    while(1) {
        
        if (OCTAVE_BUTTON_1 == 0 || OCTAVE_BUTTON_2 == 0) {
            if (OCTAVE_BUTTON_1 == 0) {
                // Octave 1 selected
                octave = 1;
                OCTAVE_LED_1 = 1;
                OCTAVE_LED_2 = 0;
            } else if (OCTAVE_BUTTON_2 == 0) {
                // Octave 2 selected
                octave = 2;
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
            } else {
                // Display Nothing
                shiftOut(DisplayValues[7]);
            }
            
            // Send Note over UART
            if (NOTE != 'X') {
                WriteUART(NOTE);
            }
            WriteUART('\n');

            PREVIOUS_NOTE = NOTE;

        } else {
            octave = 0;
            // Display Nothing
            shiftOut(DisplayValues[7]);
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
    TRISBbits.TRISB13 = 0;      // Setting the TX_2 to output
    
    RPOR6bits.RP45R = 1;        // Setting TX_2 pin to UART1 Transmit
    U1MODEbits.UARTEN = 1;      // UART Enable bit
    U1MODEHbits.BCLKSEL = 0;    // Baud Clock Source Selection bits
    U1MODEbits.BRGH = 1;        // High Baud Rate Select Bit
    U1BRGbits.BRG = 34;         // Setting the baud clock to 115200
    U1MODEHbits.BCLKMOD = 0;    // Setting it to legacy mode

    U1MODEbits.MOD = 0;         // UART Mode bits -- Asynchronous 8-bit UART No parity bit
}

void setCurrentNote() {
    if (distance < 15) {
        NOTE = 'C';
    } else if (distance < 20) {
        NOTE = 'B';
    } else if (distance < 25){
        NOTE = 'A';
    } else if (distance < 30){
        NOTE = 'G';
    } else if (distance < 35){
        NOTE = 'F';
    } else if (distance < 40){
        NOTE = 'E';
    } else if (distance < 45){
        NOTE = 'D';
    } else if (distance < 50){
        NOTE = 'C';
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
    
    // Display Nothing
    DisplayValues[7][0] = 1;
    DisplayValues[7][1] = 1;
    DisplayValues[7][2] = 1;
    DisplayValues[7][3] = 1;
    DisplayValues[7][4] = 1;
    DisplayValues[7][5] = 1;
    DisplayValues[7][6] = 1;
    DisplayValues[7][7] = 1;
    
    // Display Nothing
    shiftOut(DisplayValues[7]);
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
