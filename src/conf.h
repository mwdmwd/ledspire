#ifndef CONF_H_
#define CONF_H_

#include <stdint.h>
#include <Arduino.h>
#include <EEPROM.h>

extern struct conf
{
	uint32_t _conf_magic;
	uint16_t _conf_size;
	int r, g, b;
} conf;

#define CONF_MAGIC 0xCAFED00D
#define CONF_SIZE (sizeof(struct conf))
#define CONF_WRITE_INTERVAL_MS 120*1000

bool confValid(void);
void confRead(void);
void confWrite(void);
void confUpdate(void);

#endif
