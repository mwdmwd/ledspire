#ifndef RGB_H_
#define RGB_H_

#include <stdint.h>

#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

#define SETUP_PIN(pin) do{pinMode(pin, OUTPUT);analogWrite(pin, 0);}while(0);

extern int r, g, b;

uint16_t gammaCorr8to16(uint8_t val);

void setRGB(int nr, int ng, int nb);
void saveRGB(void);
void restoreRGB(void);

#endif
