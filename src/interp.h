#ifndef INTERP_H_
#define INTERP_H_

#define MAX_PROG_LEN 2000
#define MAX_LINE_LEN 32
#define MAX_CONSECUTIVE_LINES 10

#define NUM_REGS 10
#define STACK_SIZE 10

struct interpState
{
	bool running;
	char prog[MAX_PROG_LEN];
	int progLen;
	int pc;
	int regs[NUM_REGS];

	short sp;
	int stack[STACK_SIZE];

	union
	{
		int all;
		struct
		{
			bool z:1;
			bool c:1;
			bool s:1;
			bool o:1;
		};
	} flags;

	struct
	{
		bool delaying;
		unsigned long endMillis;
	} delay;

	struct
	{
		bool fading;
		int r0, g0, b0;
		int r1, g1, b1;
		unsigned long t0;
		int dt;
	} fade;
};

bool loadProgram(const char *name);
bool startProgram(void);
void stopProgram(void);
void interpUpdate(void);

#endif 
