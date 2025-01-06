#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define COLOR_COMMON           1
#define COLOR_STEP_DEBUG       2

struct debugger {
	uint16_t offset;
	uint8_t count;
	uint8_t nl_count;
	uint8_t hbyte;
	uint8_t lbyte;
	uint8_t args;
	uint8_t instruction;
};

uint32_t is_debug_on_line (uint16_t line, uint8_t *posx, uint8_t *count);
void debug_set_step (uint8_t opcode, uint16_t offset);
void debug_input (int c);

#endif
