#include "rgb.h"
#include "conf.h"

int r, g, b;

uint16_t gammaCorr8to16(uint8_t val)
{
	return (uint16_t)(pow((float)val / 255.f, 2.8f) * 1023 + 0.5);
}

#define CLAMP(x,min,max) do{if((x)>(max))(x)=(max);else if((x)<(min))(x)=(min);}while(0)
void setRGB(int nr, int ng, int nb)
{
	CLAMP(nr, 0, 255);
	CLAMP(ng, 0, 255);
	CLAMP(nb, 0, 255);
	if(r != nr)
	{
		analogWrite(RED_PIN, gammaCorr8to16(nr)); r = nr;
	}
	if(g != ng)
	{
		analogWrite(GREEN_PIN, gammaCorr8to16(ng)); g = ng;
	}
	if(b != nb)
	{
		analogWrite(BLUE_PIN, gammaCorr8to16(nb)); b = nb;
	}
}

void saveRGB(void)
{
	conf.r = r;
	conf.g = g;
	conf.b = b;
}

void restoreRGB(void)
{
	setRGB(conf.r, conf.g, conf.b);
}
