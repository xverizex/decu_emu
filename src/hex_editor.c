#include "hex_editor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static uint32_t
hex_editor_movement (struct hex_editor *h, int c)
{
	switch (c) {
		case 'l':
			h->cursorx++;
			if (h->cursorx >= 16) h->cursorx = 15;
			break;
		case 'h':
			h->cursorx--;
			if (h->cursorx < 0) h->cursorx = 0;
			break;
		case 'j':
			h->cursory++;
			if (h->cursory >= (h->height - h->uoff)) {
				h->cursory = h->height - h->uoff - 1;
				if ((h->top_line + h->height) < 0xffff) {
					h->top_line++;
				}
			}
			break;
		case 'k':
			h->cursory--;
			if (h->cursory < 0) {
				h->cursory = 0;
				if (h->top_line > 0) {
					h->top_line--;
				}
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

	uint8_t *wr_byte = &h->bytes[h->cursory][h->cursorx];
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
				if ((h->top_line + h->height) < 0xffff) {
					h->top_line++;
				}
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
	char buf_line[512];
	char *bl = buf_line;

	int n;
	for (int x = 0; x < 16; x++) {
		snprintf (bl, 4, " %02x", h->bytes[h->top_line + index_line - 1][x]);
		bl += 3;
	}

	mvwaddstr (h->win, index_line, posx, buf_line);
}

static void
print_info (struct hex_editor *h)
{
	mvwaddstr (h->win, 1,  1, "****|  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
	mvwaddstr (h->win, 1, 56, "'m' - movement mode");
	mvwaddstr (h->win, 2, 56, "'i' - insert mode");
	mvwaddstr (h->win, 3, 56, "'q' - quit editor");
	mvwaddstr (h->win, 4, 56, "'q' - quit from game");
	mvwaddstr (h->win, 5, 56, "'r' - run simulate");
	mvwaddstr (h->win, 6, 56, "'w' - save to file");
	mvwaddstr (h->win, 7, 56, "'hjkl' - vim movement");
}

void 
hex_editor_draw (struct hex_editor *h)
{
	uint16_t addr_off = h->top_line * LENGTH_OF_BYTES_PER_LINE;
	uint16_t end_of_line = h->top_line + h->height - 1;

	uint16_t index_line = h->uoff;
	while (index_line < h->height) {
		char str_number_line[32];
		snprintf (str_number_line, 32, "%04x:", addr_off);
		mvwaddstr (h->win, index_line, h->loff, str_number_line);
		hex_editor_draw_line_bytes (h, index_line, addr_off);
		addr_off += LENGTH_OF_BYTES_PER_LINE;
		index_line++;
	}

	uint16_t px = h->lbytes + h->half_byte_pos + h->cursorx * 3 + 1;
	uint16_t py = h->uoff + h->cursory;

	print_info (h);

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
		case 'm':
			h->cur_handle_input = hex_editor_movement;
			h->half_byte_pos = 0;
			break;
		case 'i':
			h->cur_handle_input = hex_editor_insert;
			h->half_byte_pos = 0;
			break;
		case 'w':
			write_buffer (h);
			break;
		case 'r':
			h->is_simulate = 1;
			break;
		default:
			return h->cur_handle_input (h, c);
	}


	return 1;
}
