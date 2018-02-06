#include "interp.h"
#include <FS.h>
#include "rgb.h"

static struct interpState state;

bool loadProgram(const char *name)
{
	File pf = SPIFFS.open(name, "r");
	if(!pf)
		return false;
	int len = pf.available();
	if(!len || len > MAX_PROG_LEN - 1)
	{
		pf.close();
		return false;
	}
	state.progLen = len;
	pf.read((uint8_t*)state.prog, len);
	state.prog[len] = 0;
	state.pc = 0;
	pf.close();
	return true;
}

bool startProgram(void)
{
	if(!state.prog[0])
		return false;
	state.running = true;
	state.pc = 0;
	return true;
}

void stopProgram(void)
{
	state.running = false;
}

void interpUpdate(void)
{
	char interpLine[25];
	if(!state.running || !state.prog[0] || state.pc == -1)
		return;

	char *beg = &state.prog[state.pc];
	int lineLen = strcspn(beg, "\n");
	if(!lineLen)
		goto out;

	strncpy(interpLine, beg, lineLen);
	interpLine[lineLen] = 0;
	if(!strncmp(interpLine, "rgb", 3))
	{
		int rr, gg, bb;
		if(sscanf(interpLine, "rgb %d %d %d", &rr, &gg, &bb) != 3)
			goto out;
		setRGB(rr, gg, bb);
	}
	else if(!strncmp(interpLine, "wait", 4))
	{
		int time;
		sscanf(interpLine, "wait %d", &time);
		delay(time);
	}


out:
	state.pc += lineLen + 1;
	if(state.pc >= state.progLen)
		state.pc = 0;
} 
