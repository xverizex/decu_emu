#include "hex_editor.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern uint32_t is_colored;

static uint32_t
hex_editor_movement (struct hex_editor *h, int c)
{
	switch (c) {
		case 'l':
			h->cursorx++;
			if (h->cursorx >= 16) {
				h->cursorx = 0;
				goto key_j;
			}
			break;
		case 'h':
			h->cursorx--;
			if (h->cursorx < 0) {
				h->cursorx = 15;
				goto key_k;
			}
			break;
		case 'j':
key_j:
			h->cursory++;
			if (h->cursory >= (h->height - h->uoff)) {
				h->cursory = h->height - h->uoff - 1;
				h->top_line += 16;
			}
			break;
		case 'k':
key_k:
			h->cursory--;
			if (h->cursory < 0) {
				h->cursory = 0;
				h->top_line -= 16;
			}
			break;
		case 'q':
			h->is_quit = 1;
			return 0;
	}

	return 1;
}

static uint8_t
write_half_byte (struct hex_editor *h, int c)
{
	uint8_t half_byte = 0;

	if (c >= '0' && c <= '9') {
		half_byte = c - '0';
	}

	if (c >= 'A' && c <= 'F') 
		c += 32;

	if (c >= 'a' && c <= 'f') 
		half_byte = c - 'a' + 10;


	return half_byte;
}

static uint32_t
is_hex_number (int c)
{
	if (c >= '0' && c <= '9') return 1;

	if (c >= 'A' && c <= 'F') return 1;

	if (c >= 'a' && c <= 'f') return 1;

	return 0;
}

static uint32_t
hex_editor_insert (struct hex_editor *h, int c)
{
	if (!is_hex_number (c))
		return 1;

	uint8_t *b = &h->bytes[0][0];

	uint8_t *wr_byte = &b[h->top_line + (h->cursory * 16) + h->cursorx];
	if (h->half_byte_pos == 0) {
		*wr_byte &= 0x0f;
		*wr_byte |= (write_half_byte (h, c) << 4);
		h->half_byte_pos++;
	} else {
		*wr_byte &= 0xf0;
		*wr_byte |= write_half_byte (h, c) & 0x0f;
		h->half_byte_pos = 0;
		if (h->cursorx >= 15) {
			h->cursorx = 0;
			h->cursory++;
			if (h->cursory >= (h->height - 1)) {
				h->cursory = h->height - 2;
			h->top_line++;
			}
		} else {
			h->cursorx++;
		}
	}

	return 1;
}

struct hex_editor *
hex_editor_create (
		const char *filename,
		WINDOW *win,
		uint32_t width, 
		uint32_t height, 
		uint16_t lpos, 
		uint16_t upos)
{
	struct hex_editor *editor = malloc ( sizeof (struct hex_editor));
	memset (editor, 0, sizeof (struct hex_editor));

	editor->filename = filename;
	editor->win = win;
	editor->loff = lpos;
	editor->uoff = upos;
	editor->width = width - 1;
	editor->height = height - 1;
	editor->cursorx = 0;
	editor->cursory = 0;
	editor->top_line = 0;
	editor->lbytes = 6;
	editor->cur_handle_input = hex_editor_movement;
	editor->half_byte_pos = 0;
	editor->is_simulate = 0;
	editor->is_quit = 0;
	editor->is_debug = 0;
	editor->mode = MODE_MOVEMENT;

	editor->dump = fopen (filename, "r");
	fread (editor->bytes, 1, 0xffff, editor->dump);
	fclose (editor->dump);

	return editor;
}

static void
hex_editor_draw_line_bytes (struct hex_editor *h,
		uint16_t index_line,
		uint16_t addr_off)
{
	uint16_t posx = h->lbytes;
	char buf_line[4];

	uint8_t posx_debug = 0;
	uint8_t count = 0;
	uint32_t is_step = h->is_debug? is_debug_on_line (index_line - h->uoff, &posx_debug, &count): 0;

	int n;
	uint32_t first_condition = 1;

	uint8_t *b = &h->bytes[0][0];

	for (int x = 0; x < 16; x++) {
		if (is_colored && is_step && count && posx_debug <= x) {
			wattron (h->win, COLOR_PAIR (COLOR_STEP_DEBUG));
		} else if (!is_colored && is_step && count && posx_debug <= x) {
			wattron (h->win, A_BOLD);
		}

		if (is_colored && is_step && count && posx_debug <= x && first_condition) {
			first_condition = 0;
			wattroff (h->win, COLOR_PAIR (COLOR_STEP_DEBUG));
			mvwprintw (h->win, index_line, posx, " ");
			wattron (h->win, COLOR_PAIR (COLOR_STEP_DEBUG));
		} else {
			mvwprintw (h->win, index_line, posx, " ");
		}
		posx++;
		uint16_t offset = h->top_line + ((index_line - h->uoff) * 16) + x;
		mvwprintw (h->win, index_line, posx, "%02x", b[offset]);
		posx += 2;

		if (is_colored && is_step && posx_debug <= x && count) {
			count--;
			posx_debug++;
			if (count == 0) {
				wattroff (h->win, COLOR_PAIR (COLOR_STEP_DEBUG));
				is_step = 0;
			}
		} else if (!is_colored && is_step && posx_debug <= x && count) {
			count--;
			posx_debug++;
			if (count == 0) {
				wattroff (h->win, A_BOLD);
				is_step = 0;
			}
		}
	}
}

static void
print_info (struct hex_editor *h)
{
	mvwprintw (h->win, 1,  1, "****|  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
	mvwprintw (h->win, 1, 56, "'m' - movement mode");
	mvwprintw (h->win, 2, 56, "'i' - insert mode");
	mvwprintw (h->win, 3, 56, "'q' - quit mode");
	mvwprintw (h->win, 4, 56, "'r' - run simulate");
	mvwprintw (h->win, 5, 56, "'w' - save to file");
	mvwprintw (h->win, 6, 56, "'hjkl' - vim movement");
	mvwprintw (h->win, 7, 56, "'d' - run in debug");
	mvwprintw (h->win, 8, 56, "'space' - step debug");
}

void 
hex_editor_draw (struct hex_editor *h)
{
	uint16_t addr_off = h->top_line;
	uint16_t end_of_line = h->top_line + (h->height - 1) * 16;

	uint16_t index_line = h->uoff;
	while (index_line < h->height) {
		mvwprintw (h->win, index_line, h->loff, "%04x:", addr_off);
		hex_editor_draw_line_bytes (h, index_line, addr_off);
		addr_off += LENGTH_OF_BYTES_PER_LINE;
		index_line++;
	}

	uint16_t px = h->lbytes + h->half_byte_pos + h->cursorx * 3 + 1;
	uint16_t py = h->uoff + h->cursory;

	print_info (h);

	h->px = px;
	h->py = py;

	wmove (h->win, py, px);
	wrefresh (h->win);
}

static void
write_buffer (struct hex_editor *h)
{
	h->dump = fopen (h->filename, "w");
	fwrite (h->bytes, 1, 0xffff, h->dump);
	fclose (h->dump);
}

uint32_t
hex_editor_input (struct hex_editor *h, int c)
{
	uint16_t px, py;

	switch (c) {
		case KEY_NPAGE:
			if (h->mode == MODE_MOVEMENT)
				h->top_line += (h->height - h->uoff) * 16;
			break;
		case KEY_PPAGE:
			if (h->mode == MODE_MOVEMENT)
				h->top_line -= (h->height - h->uoff) * 16;
			break;
		case 'm':
			h->mode = MODE_MOVEMENT;
			h->cur_handle_input = hex_editor_movement;
			h->half_byte_pos = 0;
			mvwprintw (h->win, 0, 16, " mode: movement ------");
			wrefresh (h->win);
			break;
		case 'i':
			h->mode = MODE_EDITING;
			h->cur_handle_input = hex_editor_insert;
			mvwprintw (h->win, 0, 16, " mode: inserting ------");
			wrefresh (h->win);
			h->half_byte_pos = 0;
			break;
		case 'w':
			write_buffer (h);
			break;
		case 'r':
			h->mode = MODE_SIMULATION;
			mvwprintw (h->win, 0, 16, " mode: simulation ------");
			wrefresh (h->win);
			h->is_simulate = 1;
			break;
		case 'd':
			if (h->mode != MODE_EDITING && h->mode != MODE_SIMULATION) {
				mvwprintw (h->win, 0, 16, " mode: debug ------");
				wrefresh (h->win);
				h->is_debug = 1;
				h->is_simulate = 1;
			} else if (h->mode == MODE_EDITING) {
				return h->cur_handle_input (h, c);
			}
			break;
		default:
			return h->cur_handle_input (h, c);
	}


	return 1;
}
