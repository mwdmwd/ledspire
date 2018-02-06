#ifndef INTERP_H_
#define INTERP_H_

#define MAX_PROG_LEN 1500

struct interpState
{
	bool running;
	char prog[MAX_PROG_LEN];
	int progLen;
	int pc;
};

bool loadProgram(const char *name);
bool startProgram(void);
void stopProgram(void);
void interpUpdate(void);

#endif 
