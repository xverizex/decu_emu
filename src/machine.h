#ifndef MACHINE_H
#define MACHINE_H
#include <stdint.h>
#include <ncurses.h>
#include <time.h>
#include "hex_editor.h"

#define MAX_WINDOW_GAME_WIDTH              20
#define MAX_WINDOW_GAME_HEIGHT             10

#define CPU_FLAG_Z                          1
#define CPU_FLAG_S                          2
#define CPU_FLAG_C                          4

enum {
	ADD,
	SUB,
	AND,
	OR,
	XOR,
	SHL,
	SHR,
	LD,
	IN,
	NOP,
	OUT,
	PUSH,
	POP,
	TEST,
	JC,
	HLT,
	N_OPERATORS
};

enum {
	REG_A,
	REG_X,
	REG_Y,
	REG_ADDR
};

enum {
	ADDR_TIMER,
	ADDR_SCREEN,
	ADDR_CROSS,
	ADDR_BUTTONS,
	N_ADDR
};

enum {
	JC_C,
	JC_S,
	JC_Z,
	JC_JMP
};

struct cpu {
	uint8_t flags;

	uint8_t a;
	uint8_t x;
	uint8_t y;

	uint16_t s;

	uint8_t ip;
};

struct machine {
	struct cpu cpu;
	uint8_t is_run;

	struct timespec timer;
	WINDOW *game_screen;
	struct hex_editor *hex_editor;
	uint8_t cross[4];
	uint8_t button[4];
};

struct machine *machine_init (WINDOW *screen_win);
void machine_run (struct machine *m);
void machine_input (struct machine *m, int c);
void machine_step_instruction (struct machine *m);

#endif
