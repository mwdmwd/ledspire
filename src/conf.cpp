#include "conf.h"

struct conf conf =
{
	._conf_magic = CONF_MAGIC,
	._conf_size = CONF_SIZE,
	.r = 0,
	.g = 255,
	.b = 0,
};

bool confValid(void)
{
	uint32_t magic;
	uint16_t size;
	for(int i = 0; i < 4; ++i)
	{
		((char*)&magic)[i] = EEPROM.read(i);
	}
	if(magic != CONF_MAGIC)
		return false;
	size = EEPROM.read(5) << 8 | EEPROM.read(4);
	return (size == CONF_SIZE);
}

void confRead(void)
{
	for(int i = 0; i < CONF_SIZE; ++i)
	{
		((char*)&conf)[i] = EEPROM.read(i);
	}
}

void confWrite(void)
{
	for(int i = 0; i < CONF_SIZE; ++i)
	{
		EEPROM.write(i, ((char*)&conf)[i]);
	}
	EEPROM.commit();
}

void confUpdate(void)
{
	static unsigned long lastWriteMillis = millis();
	unsigned long curMillis = millis();
	if(curMillis - lastWriteMillis >= CONF_WRITE_INTERVAL_MS)
	{
		/* We don't have to worry about using up write cycles because
		 * the EEPROM library only actually writes to the "EEPROM" if
		 * something has changed */
		confWrite();
		lastWriteMillis = curMillis;
	}
}
