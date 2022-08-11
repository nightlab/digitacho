#include <Arduino.h>
#include "lcd.h"
#include "adc.h"

static float voltConversionTab[6] = {
    8.0f, 9.0f, 10.0f, 12.0f, 14.0f, 15.0f
};

static float voltFuel, voltOil, voltBattery, voltTemp;

static unsigned short speed = 0;
static unsigned short rpm = 0;

static byte oilCritical = 0, batteryCritical = 0, tempCritical = 0, fuelCritical = 0;

static byte loopCounter;

static byte oilDummy = 0;
static byte fuelDummy = 0;
static byte tempDummy = 0;
static byte batDummy = 0;

void updateBattery() {
    byte a = 0;
  
    for(byte x=0;x<6;x++) {
        if(voltBattery >= voltConversionTab[x])
            a = x + 1;
    }
    lcdRenderBattery(a);
  
    // Check warnings
    if(voltBattery < 9.0f) {
        batteryCritical = 1;
    } else {
        //DISABLE_BATTERYWARNING();
        batteryCritical = 0;
    }
}

void updateWarnings() {
    if (oilCritical) {
        //TOGGLE_OILWARNING();
    }
    if (batteryCritical) {
        //TOGGLE_BATTERYWARNING();
    }
    if (tempCritical) {
        //TOGGLE_TEMPWARNING();
    }
    if (fuelCritical) {
        //TOGGLE_FUELWARNING();
    }
}

void setup() {
    lcdInit();
    adcInit();

    Serial.begin(SERIAL_BAUDRATE);
    Serial.write("Ready\n");

    voltBattery = 13.7f;
}

void loop() {

    lcdSetTachFrame(true);
    lcdSetSpeedFrame(true);
    lcdSetStatusFrame(true);

    updateBattery();

    lcdRenderBattery(batDummy);
    lcdRenderOil(oilDummy);
    lcdRenderFuel(fuelDummy);
    lcdRenderTemperature(tempDummy);
    lcdRenderSpeed(speed);
    lcdRenderRPM(rpm);

    lcdSendBuffer();

    speed = 99;
    speed++;
    if (speed > 298)
        speed = 0;

    rpm += 100;
    if (rpm > 8000)
        rpm = 0;
    rpm = 3500;

    oilDummy = 3;
    fuelDummy = 6;
    //tempDummy = 3;
    batDummy = 3;
    
    delay(50);
    loopCounter++;
    if (loopCounter > 9) {
        loopCounter = 0;
        //updateWarnings();

        /*if(++oilDummy > 6)
            oilDummy = 0;
        if(++fuelDummy > 13)
            fuelDummy = 0;
        if(++tempDummy > 6)
            tempDummy = 0;
        if(++batDummy > 6)
            batDummy = 0;*/

        unsigned short x = adcAcquire(0);
        Serial.print("[0] ");
        Serial.print(x, DEC);
        Serial.print("\t[1] ");

        x = adcAcquire(1);
        Serial.print(x, DEC);
        Serial.print("\t[2] ");
        tempDummy = x / 8192;

        x = adcAcquire(2);
        Serial.print(x, DEC);
        Serial.print("\t[3] ");

        x = adcAcquire(3);
        Serial.print(x, DEC);
        Serial.print("\t[4] ");

        x = adcAcquire(4);
        Serial.print(x, DEC);
        Serial.print("\t[5] ");

        x = adcAcquire(5);
        Serial.print(x, DEC);
        Serial.print("\n");
    }
}
