#ifndef ADC_H
#define ADC_H

#include "Arduino.h"
#include "config.h"

extern void adcInit();

extern unsigned short adcAcquire(byte channel);

#endif
