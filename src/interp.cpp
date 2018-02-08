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
	memset(&state.delay, 0, sizeof(state.delay));
	memset(&state.fade, 0, sizeof(state.fade));
	memset(&state.regs, 0, sizeof(state.regs));
	return true;
}

void stopProgram(void)
{
	state.running = false;
}

static bool runLine(void);
void interpUpdate(void)
{
	/* Check if running and whether state looks reasonable */
	if(!state.running || !state.prog[0] || state.pc == -1)
		return;

	/* If we are in a 'wait' command, no need to update anything */
	if(state.delay.delaying && state.delay.endMillis > millis())
		return;
	else if(state.delay.delaying) /* wait just expired? */
		state.delay.delaying = false; /* Mark to short-circuit above comparison next time */

	/* Handle 'fade' effect */
	if(state.fade.fading)
	{
		int tx = millis() - state.fade.t0;
		if(tx >= state.fade.dt)
		{
			setRGB(state.fade.r1, state.fade.g1, state.fade.b1);
			state.fade.fading = false; /* Fade ended, need to run now */
		}
		else
		{
			int nr = state.fade.r0 + (state.fade.r1-state.fade.r0)*tx/state.fade.dt;
			int ng = state.fade.g0 + (state.fade.g1-state.fade.g0)*tx/state.fade.dt;
			int nb = state.fade.b0 + (state.fade.b1-state.fade.b0)*tx/state.fade.dt;
			setRGB(nr, ng, nb);
			return;
		}
	}

	int i = MAX_CONSECUTIVE_LINES;
	while(i-- && !runLine());
}

/* Returns TRUE if blocked, FALSE if another line can be ran */
static bool runLine(void)
{
	char interpLine[MAX_LINE_LEN];
	bool block = false;

	char *beg = &state.prog[state.pc];
	int lineLen = strcspn(beg, "\n");
	if(!lineLen || lineLen > MAX_LINE_LEN)
		goto out;

	strncpy(interpLine, beg, lineLen);
	interpLine[lineLen] = 0;
	if(interpLine[0] == ':')
		goto out; // Label line
	if(!strncmp(interpLine, "rgb", 3))
	{
		int rr, gg, bb;
		if(sscanf(interpLine, "rgb %d %d %d", &rr, &gg, &bb) == 3)
			setRGB(rr, gg, bb);
	}
	else if(!strncmp(interpLine, "wait", 4))
	{
		int time;
		if(sscanf(interpLine, "wait %d", &time) == 1)
		{
			state.delay.delaying = true;
			state.delay.endMillis = millis() + time;
			block = true;
		}
	}
	else if(!strncmp(interpLine, "fade", 4))
	{
		if(sscanf(interpLine, "fade %d %d %d %d",
		          &state.fade.r1, &state.fade.g1, &state.fade.b1,
		          &state.fade.dt) == 4)
		{
			state.fade.r0 = r;
			state.fade.g0 = g;
			state.fade.b0 = b;
			state.fade.fading = true;
			block = true;
		}
	}

out:
	state.pc += lineLen + 1;
	if(state.pc >= state.progLen)
		state.pc = 0;
	return block;
} 
