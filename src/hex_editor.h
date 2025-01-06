#ifndef HEX_EDITOR_GAME_H
#define HEX_EDITOR_GAME_H

#include <ncurses.h>
#include <stdint.h>

#define MAX_BYTES_HEX_EDITOR          65535
#define MAX_LINES_IN_BYTES             4096
#define MAX_BYTES_IN_LINE                16
#define LENGTH_OF_BYTES_PER_LINE         16

struct hex_editor {
	uint16_t top_line;
	uint32_t width;
	uint32_t height;
	int16_t cursorx;
	int16_t cursory;
	uint16_t loff;
	uint16_t uoff;
	uint16_t lbytes;
	WINDOW *win;
	uint8_t bytes[MAX_LINES_IN_BYTES][MAX_BYTES_IN_LINE];
	FILE *dump;
	uint32_t (*cur_handle_input) (struct hex_editor *self, int sym);
	uint8_t half_byte_pos;
	const char *filename;
	uint32_t is_simulate;
	uint32_t is_debug;
	uint8_t is_quit;
	uint16_t px;
	uint16_t py;
	uint32_t mode;
};

struct hex_editor *hex_editor_create (
		const char *filename,
		WINDOW *win,
		uint32_t width, 
		uint32_t height, 
		uint16_t lpos, 
		uint16_t upos);

enum {
	MODE_MOVEMENT,
	MODE_EDITING,
	MODE_DEBUGGING,
	MODE_SIMULATION,
	N_MODE
};

void hex_editor_draw (struct hex_editor *self);
uint32_t hex_editor_input (struct hex_editor *h, int c);

#endif
