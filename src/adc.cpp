#include <Arduino.h>
#include "adc.h"

/*
 * Initialize pins
 */
void adcInit() {
    pinMode(PIN_ADC_CLK, OUTPUT);
    pinMode(PIN_ADC_CS, OUTPUT);
    pinMode(PIN_ADC_DIDO, INPUT);

    digitalWrite(PIN_ADC_CLK, HIGH);
    digitalWrite(PIN_ADC_CS, HIGH);

    //delayMicroseconds(5);
    //digitalWrite(PIN_LCD_BLANK, LOW);
}

byte __readBit() {
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);
    byte v = digitalRead(PIN_ADC_DIDO);
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
    return v;
}

void __clock() {
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
}

unsigned short adcAcquire(byte channel) {
    pinMode(PIN_ADC_DIDO, OUTPUT); // switch data to output
    digitalWrite(PIN_ADC_DIDO, HIGH); // start bit
    digitalWrite(PIN_ADC_CS, LOW); // chip select
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);

    // Cycle 1
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_ADC_DIDO, HIGH); // SGL
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);

    // Cycle 2
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_ADC_DIDO, (channel >> 2) & 0x01); // bit 2
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);

    // Cycle 3
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_ADC_DIDO, (channel >> 1) & 0x01); // bit 1
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);

    // Cycle 4
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_ADC_DIDO, channel & 0x01); // bit 0
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);

    // Cycle 5
    digitalWrite(PIN_ADC_CLK, LOW);
    delayMicroseconds(5);
    
    pinMode(PIN_ADC_DIDO, INPUT); // switch data to input
    
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(5);

    unsigned short v = 0;
    for (byte x=0;x<16;x++) {
        v |= __readBit();
        v <<= 1;
    }

    digitalWrite(PIN_ADC_CS, HIGH);
    digitalWrite(PIN_ADC_CLK, HIGH);
    delayMicroseconds(25);

    return v;
}