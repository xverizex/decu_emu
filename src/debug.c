#include "debug.h"
#include "machine.h"
#include <stdlib.h>
#include <ncurses.h>

static struct debugger debugger;
extern struct machine *machine;
extern struct hex_editor *hex_editor;

uint32_t
is_debug_on_line (uint16_t line, uint8_t *posx, uint8_t *count)
{
	uint16_t start = debugger.offset / 16;
	uint16_t end = (debugger.offset + debugger.count) / 16;

	uint16_t off = 16;
	line = (hex_editor->top_line + line * off);
	line /= off;
	if (line >= start && line <= end) {
		if (start != end) {
			uint16_t d_count = 0;
			uint8_t px = debugger.offset % 16;

			if (px == 0xf && debugger.count == 1 && line == start) {
				*posx = px;
				d_count = 1;
			} else if (debugger.count > 1) {
				if (line == start) {
					d_count = (16 * (line + 1)) - debugger.offset;
					debugger.nl_count = debugger.count - d_count;
					*posx = debugger.offset;
				} else if (line == end) {
					d_count = debugger.nl_count;
					*posx = 0;
				}
			}

			*count = d_count;
		} else {
			*posx = debugger.offset % 16;
			*count = debugger.count;
		}
		return 1;
	}

	return 0;
}

void
debug_set_step (uint8_t opcode, uint16_t offset)
{
	uint8_t instruction = opcode >> 4;
	uint8_t args = opcode & 0x0f;
	uint8_t hbyte = args >> 2;
	uint8_t lbyte = args &  3;
	/*
	 * For tests
	 */
	debugger.hbyte = hbyte;
	debugger.lbyte = lbyte;
	debugger.args = args;
	debugger.instruction = instruction;

	if (instruction == HLT || instruction == NOP) {
		debugger.offset = offset;
		debugger.count = 1;
		return;
	}
	if (instruction == JC) {
		debugger.offset = offset;
		debugger.count = 3;
		return;
	}


	if (args == 0xf) {
		debugger.offset = offset;
		debugger.count = 3;
		return;
	}

	if (hbyte == 0x3 || lbyte == 0x3) {
		debugger.offset = offset;
		debugger.count = 2;
		return;
	}

	if (instruction == PUSH || instruction == POP) {
		debugger.offset = offset;
		debugger.count = 1;
		return;
	}

	debugger.offset = offset;
	debugger.count = 2;
	return;
}

void
debug_set_windows (WINDOW *cpu, WINDOW *stack)
{
	debugger.cpu = cpu;
	debugger.stack = stack;
}

void
debug_input (int c)
{
	switch (c) {
		case 'q':
			hex_editor->mode = MODE_MOVEMENT;
			hex_editor->is_debug = 0;
			hex_editor->is_simulate = 0;
			machine->is_run = 0;
			machine->cpu.ip = 0;
			mvwprintw (hex_editor->win, 0, 16, " mode: movement ------");
			wrefresh (hex_editor->win);
			break;
		case ' ':
			hex_editor->is_simulate = 1;
			machine_step_instruction (machine);
			break;
	}
}

void
debug_print_info ()
{
	char line[64];
	mvwprintw (debugger.cpu, 1, 2, "FLAGS: %04x", machine->cpu.flags);
	mvwprintw (debugger.cpu, 2, 2, "   IP: %04x", machine->cpu.ip);
	mvwprintw (debugger.cpu, 3, 2, "    A: %02x", machine->cpu.a);
	mvwprintw (debugger.cpu, 4, 2, "    X: %02x", machine->cpu.x);
	mvwprintw (debugger.cpu, 5, 2, "    Y: %02x", machine->cpu.y);
	mvwprintw (debugger.cpu, 6, 2, "    S: %04x", machine->cpu.s);
	wrefresh (debugger.cpu);

	int y_height = 1;
	uint8_t *b = &hex_editor->bytes[0][0];

	uint16_t end_stack_line = machine->cpu.s;
	uint16_t top_line = 0xffff;

	uint16_t nline = (uint16_t) top_line - end_stack_line;
	uint16_t stack = top_line;

	if (nline >= ((DEBUG_WINDOW_HEIGHT - 2) / 2)) {
		stack = end_stack_line + 5;
	}

	for (int y = 0; y < DEBUG_WINDOW_HEIGHT - 2; y++) {

		uint16_t byte = b[stack];
		mvwprintw (debugger.stack, y_height, 1, "%04x: %02x", stack, byte);
		y_height++;
		stack--;
	}

	wrefresh (debugger.stack);
}
