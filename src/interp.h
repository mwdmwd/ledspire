#ifndef INTERP_H_
#define INTERP_H_

#define MAX_PROG_LEN 1500
#define MAX_LINE_LEN 32

struct interpState
{
	bool running;
	char prog[MAX_PROG_LEN];
	int progLen;
	int pc;

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
