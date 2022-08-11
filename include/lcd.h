#ifndef LCD_H
#define LCD_H

#include "Arduino.h"
#include "config.h"

extern void lcdInit();

extern void lcdRenderSpeed(unsigned short speed);
extern void lcdRenderTemperature(byte value);
extern void lcdRenderFuel(byte value);
extern void lcdRenderBattery(byte value);
extern void lcdRenderOil(byte value);
extern void lcdRenderRPM(unsigned short rpm);

extern void lcdSetTachFrame(boolean enabled);
extern void lcdSetSpeedFrame(boolean enabled);
extern void lcdSetStatusFrame(boolean enabled);

extern void lcdSendBuffer();

#endif
