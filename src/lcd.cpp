#include <Arduino.h>
#include "lcd.h"

#define SWITCH_ON(xfg, yfg) \
    lcdBuffer[xfg] |= yfg;

#define SWITCH_OFF(xfg, yfg) \
    lcdBuffer[xfg] &= ~yfg;

#define TOGGLE(xfg, yfg) \
    if (lcdBuffer[xfg] & yfg) { SWITCH_OFF(xfg, yfg) } else { SWITCH_ON(xfg, yfg) }

#define ENABLE_OILWARNING()         SWITCH_ON(7, 0x01)
#define DISABLE_OILWARNING()        SWITCH_OFF(7, 0x01)
#define TOGGLE_OILWARNING()         TOGGLE(7, 0x01)

#define ENABLE_BATTERYWARNING()     SWITCH_ON(7, 0x40)
#define DISABLE_BATTERYWARNING()    SWITCH_OFF(7, 0x40)
#define TOGGLE_BATTERYWARNING()     TOGGLE(7, 0x40)

#define ENABLE_TEMPWARNING()        SWITCH_ON(8, 0x20)
#define DISABLE_TEMPWARNING()       SWITCH_OFF(8, 0x20)
#define TOGGLE_TEMPWARNING()        TOGGLE(8, 0x20)

#define ENABLE_FUELWARNING()        SWITCH_ON(9, 0x10)
#define DISABLE_FUELWARNING()       SWITCH_OFF(9, 0x10)
#define TOGGLE_FUELWARNING()        TOGGLE(9, 0x10)


static const byte lcdCharTab[10] = {
    0b01101111, 0b00000011, 0b01110110, 0b01010111, 0b00011011,
    0b01011101, 0b01111101, 0b00000111, 0b01111111, 0b00011111
};

static const unsigned short rpmBarTab[32] = {
    750,
    1000, 1200, 1400, 1600, 1800, 2000, 2200, 2400,
    2600, 2800, 3000, 3200, 3400, 3600, 3800, 4000,
    4200, 4400, 4600, 4800, 5000, 5750, 5500, 5250,
    6000, 6325, 6650, 7000, 7325, 7650, 8000
};

static byte lcdBuffer[12] = { 0, };


static void _renderM1(byte c) {
    if (c > 10) {
        lcdBuffer[0] &= 0b00000111;
        lcdBuffer[1] &= 0b11111100;
        return;
    }
    byte temp = lcdBuffer[0] & 0x07;
    temp |= lcdCharTab[c] << 3;
    lcdBuffer[0] = temp;
    temp = lcdBuffer[1] & 0xFC;
    temp |= lcdCharTab[c] >> 5;
    lcdBuffer[1] = temp;
}

static void _renderM2(byte c) {
    if(c > 10) {
        lcdBuffer[1] &= 0b11000011;
        lcdBuffer[6] &= 0b11111000;
        return;
    }
    byte temp = lcdBuffer[1] & 0xC3;
    temp |= (lcdCharTab[c] << 2) & 0x3C;
    lcdBuffer[1] = temp;
    temp = lcdBuffer[6] & 0xF8;
    temp |= lcdCharTab[c] >> 4;
    lcdBuffer[6] = temp;
}

static void _renderM3(byte c) {
    byte temp;
    switch(c) {
        case 1:
            temp = lcdBuffer[6] & 0b11000111;
            temp |= 0b00011000;
            lcdBuffer[6] = temp;
            break;
        case 2:
            temp = lcdBuffer[6] & 0b11000111;
            temp |= 0b00110000;
            lcdBuffer[6] = temp;
            break;
        default:
            lcdBuffer[6] &= 0b11000111;
    }
}

/*
 * Initialize output pins and setup display
 */
void lcdInit() {
    pinMode(PIN_LCD_CLOCK, OUTPUT);
    pinMode(PIN_LCD_DATA, OUTPUT);
    pinMode(PIN_LCD_LOAD, OUTPUT);
    pinMode(PIN_LCD_BLANK, OUTPUT);

    delayMicroseconds(5);
    digitalWrite(PIN_LCD_BLANK, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_LCD_BLANK, LOW);
    delayMicroseconds(5);

    lcdSendBuffer();
}

/*
 * Enable or disable tach frame
 */
void lcdSetTachFrame(boolean enabled) {
    if (enabled)
        SWITCH_ON(5, 0x10)
    else
        SWITCH_OFF(5, 0x10)
}

/*
 * Enable or disable speed frame (km/h display)
 */
void lcdSetSpeedFrame(boolean enabled) {
    if (enabled)
        SWITCH_ON(0, 0x04)
    else
        SWITCH_OFF(0, 0x04)
}

/*
 * Enable or disable status frame
 */
void lcdSetStatusFrame(boolean enabled) {
    if (enabled)
        SWITCH_ON(6, 0x80)
    else
        SWITCH_OFF(6, 0x80)
}

/*
 * Render speed gauge
 *
 * speed: 0-299 (0 = off)
 */
void lcdRenderSpeed(unsigned short speed)
{
    if(speed > 299)
        speed = 299;

    byte m3 = speed / 100;
    byte m2 = (speed / 10) - (m3 * 10);
    byte m1 = speed % 10;

    if (speed < 10) {
        _renderM1(m1);
        _renderM2(11);
        _renderM3(0);
    } else {
        _renderM1(m1);
        _renderM2(m2);
        _renderM3(m3);
    }
}

/*
 * Render temperature gauge
 *
 * value: 0-6 (0 = off)
 */
void lcdRenderTemperature(byte value) {
    if (value > 6)
        value = 6;

    byte v = 0, x;
    for (x=0;x<value;x++) {
        v |= 1 << x;
    }
    x = lcdBuffer[8] & 0b00111111;
    x |= v << 6;
    lcdBuffer[8] = x;
    x = lcdBuffer[9] & 0b11110000;
    x |= v >> 2;
    lcdBuffer[9] = x;
}

/*
 * Render fuel gauge
 *
 * value: 0-13 (0 = off)
 */
void lcdRenderFuel(byte value) {
    if(value > 13)
        value = 13;

    byte x;
    unsigned short v = 0;
    for (x=0;x<value;x++) {
        v |= 1 << x;
    }
    x = lcdBuffer[9] & 0b00011111;
    lcdBuffer[9] = x | (v << 5);
    lcdBuffer[10] = v >> 3;
    x = lcdBuffer[11] & 0b11111110;
    lcdBuffer[11] = x | (v >> 11);
}

/*
 * Render battery gauge
 *
 * value: 0-6 (0 = off)
 */
void lcdRenderBattery(byte value) {
    if(value > 6)
        value = 6;

    byte x, v = 0;
    for (x=0;x<value;x++) {
        v |= 1 << x;
    }
    x = lcdBuffer[7] & 0b01111111;
    lcdBuffer[7] = x | (v << 7);
    x = lcdBuffer[8] & 0b11100000;
    lcdBuffer[8] = x | (v >> 1);
}

/*
 * Render oil gauge
 *
 * value: 0-5 (0 = off)
 */
void lcdRenderOil(byte value) {
    if(value > 5)
        value = 5;

    byte x, v = 0;
    for (x=0;x<value;x++) {
        v |= 1 << x;
    }
    x = lcdBuffer[7] & 0b11000001;
    lcdBuffer[7] = x | (v << 1);
}

/*
 * Render rpm gauge
 *
 * rpm: 0-8000
 */
void lcdRenderRPM(unsigned short rpm) {
    // Clear RPM in buffer
    lcdBuffer[1] &= 0b00111111;
    lcdBuffer[2] = lcdBuffer[3] = lcdBuffer[4] = 0;
    lcdBuffer[5] = 0x10;

    // No display if below 500 rpm
    if(rpm < 500)
        return;
    
    // Enable 500 rpm bars
    lcdBuffer[1] |= 0x40;

    // Convert rpm to bitstream using RPM translation table
    unsigned int bits = 0;
    for(byte x=0;x<32;x++) {
        if (rpm >= rpmBarTab[x])
            bits |= 1 << x;
    }

    // Write bitstream to render buffer
    lcdBuffer[1] |= (bits & 0x01) << 7; bits >>= 1;
    lcdBuffer[2] |= bits; bits >>= 8;
    lcdBuffer[3] |= bits; bits >>= 8;
    lcdBuffer[4] |= bits & 0x1f; bits >>= 5;
    lcdBuffer[5] |= (bits & 0x7) << 5; bits >>= 3;
    lcdBuffer[4] |= (bits & 0x7) << 5; bits >>= 3;
    lcdBuffer[5] |= (bits & 0xf); bits >>= 4;
}

/*
 * Send render buffer to lcd display (dual OKI M5219B)
 */
void lcdSendBuffer() {
    for(byte y=0;y<12;y++) {
        for(byte x=0;x<8;x++) {
            if ((lcdBuffer[y] >> x) & 0x01)
                digitalWrite(PIN_LCD_DATA, HIGH);
            else
                digitalWrite(PIN_LCD_DATA, LOW);
            delayMicroseconds(1);
            
            digitalWrite(PIN_LCD_CLOCK, HIGH);
            delayMicroseconds(1);
            
            digitalWrite(PIN_LCD_CLOCK, LOW);
            delayMicroseconds(2);
        }
    }
    
    digitalWrite(PIN_LCD_LOAD, HIGH);
    delayMicroseconds(1);

    digitalWrite(PIN_LCD_LOAD, LOW);
    delayMicroseconds(10);
}