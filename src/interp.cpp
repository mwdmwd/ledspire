#include "interp.h"
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
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
	state.sp = 0;
	memset(&state.delay, 0, sizeof(state.delay));
	memset(&state.fade, 0, sizeof(state.fade));
	memset(&state.regs, 0, sizeof(state.regs));
	memset(&state.stack, 0, sizeof(state.stack));
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

#define VALID_REG(n) (((n)>=0)&&((n)<NUM_REGS))
int regVal(const char *str)
{
	int regNr = 0;
	if(sscanf(str, " v%d", &regNr) == 1 && VALID_REG(regNr))
		return state.regs[regNr];
	else
		return atoi(str); // Support immediate values
}

int *regRef(const char *str)
{
	int regNr = 0;
	if(sscanf(str, " v%d", &regNr) == 1 && VALID_REG(regNr))
		return &state.regs[regNr];
	return NULL;
}

/* May blow up on bad format strings
   (so don't provide bad format strings.) */
int sscanrv(const char *str, const char* format, ...)
{
	va_list ap;
	int fmts = 0;
	while(*str && isspace(*str)) ++str; // Skip leading whitespace in str
	if(!*str) return 0;
	va_start(ap, format);

	while(*format && *str)
	{
		switch(*format)
		{
			case ' ':
				while(*str && isspace(*str))
					++str;
				if(!*str) goto out;
				break;
			case '%':
				++format;
				if(*format == 'r') // Reg ref
				{
					int *ref = regRef(str);
					if(ref)
					{
						*(va_arg(ap, int**)) = ref;
						++fmts;
						while(*str && !isspace(*str)) ++str;
						if(!*str) goto out;
					}
				}
				else if(*format == 'v') // Reg val or imm val
				{
					int v = regVal(str);
					*(va_arg(ap, int*)) = v;
					++fmts;
					while(*str && !isspace(*str)) ++str;
						if(!*str) goto out;
				}
				break;
			default:
				if(*str != *format) goto out;
				++str;
				break;
		}
		++format;
	}
out:
	va_end(ap);
	return fmts;
}

void push(int what)
{
	if(state.sp < STACK_SIZE - 1)
		state.stack[++state.sp] = what;
	else
		; // TODO stack overflow error
}

int pop(void)
{
	if(state.sp > 0)
		return state.stack[state.sp--];
	else
		; // TODO stack underflow error
}

int nextLinePos(int lpos)
{
	char *nextNl = strchr(&state.prog[lpos], '\n');
	if(!nextNl) return 0;
	return (nextNl - state.prog) + 1;
}

int getLabelDest(const char *name)
{
	int pos, nlen = strlen(name);
	char *lb = state.prog;

	while(lb)
	{
		lb = strchr(lb, ':');
		if(!lb) break;
		if(lb[-1] == '\n' || lb == state.prog) // At start of line
		{
			if(!strncmp(&lb[1], name, nlen))
				return nextLinePos(lb-state.prog);
		}
		++lb;
	}
	return 0; //TODO raise error if not found
}

#define CMD(name) else if(!strncmp(interpLine, (name), sizeof(name)-1) && (args=&interpLine[sizeof(name)-1]))

#define JMP() do{state.pc = getLabelDest(args);goto out_noinc;}while(0)
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
	if(interpLine[0] == ':' || interpLine[0] == '#')
		goto out; // Label or comment

	char *args;
	if(false); // Dummy
	CMD("wait")
	{
		int time;
		if(sscanrv(args, "%v", &time) == 1)
		{
			state.delay.delaying = true;
			state.delay.endMillis = millis() + time;
			block = true;
		}
	}
	CMD("fade")
	{
		if(sscanrv(args, "%v %v %v %v",
		          &state.fade.r1, &state.fade.g1, &state.fade.b1,
		          &state.fade.dt) == 4)
		{
			state.fade.t0 = millis();
			state.fade.r0 = r;
			state.fade.g0 = g;
			state.fade.b0 = b;
			state.fade.fading = true;
			block = true;
		}
	}
	CMD("stop")
	{
		stopProgram();
		block = true;
	}
	CMD("rand")
	{
		int *dest, min, max;
		if(sscanrv(args, "%r %v %v", &dest, &min, &max) == 3)
		{
			*dest = rand() % (max-min+1) + min;
		}
	}
	CMD("rgb")
	{
		int rr, gg, bb;
		if(sscanrv(args, "%v %v %v", &rr, &gg, &bb) == 3)
			setRGB(rr, gg, bb);
	}
	CMD("add")
	{
		int *dest, addend;
		if(sscanrv(args, "%r %v", &dest, &addend) == 2)
			*dest += addend;
	}
	CMD("sub")
	{
		int *dest, subtractend; // (?)
		if(sscanrv(args, "%r %v", &dest, &subtractend) == 2)
			*dest -= subtractend;
	}
	CMD("mul")
	{
		int *dest, multiplicand;
		if(sscanrv(args, "%r %v", &dest, &multiplicand) == 2)
			*dest *= multiplicand;
	}
	CMD("div")
	{
		int *dest, denominator;
		if(sscanrv(args, "%r %v", &dest, &denominator) == 2)
			*dest /= denominator;
	}
	CMD("jmp")
	{
		JMP();
	}
	CMD("ld")
	{
		int *dest, val;
		if(sscanrv(args, "%r %v", &dest, &val) == 2)
			*dest = val;
	}

out:
	state.pc += lineLen + 1;
	if(state.pc >= state.progLen)
		state.pc = 0;
out_noinc:
	return block;
} 
