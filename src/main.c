#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include "hex_editor.h"
#include "machine.h"
#include "debug.h"
#include <time.h>

struct hex_editor *hex_editor;
struct machine *machine;

uint32_t is_colored;

static WINDOW *
game_create_window (int w, int h, int x, int y)
{
	WINDOW *win = newwin (h, w, y, x);
	box (win, '|', '-');
	return win;
}

int 
main (int argc, char **argv)
{
	if (argc < 2) {
		fprintf (stderr, "game [filename]\n");
		exit (EXIT_FAILURE);
	}

	setlocale (LC_ALL, "");
	initscr ();
	keypad (stdscr, FALSE);
	cbreak ();
	noecho ();

	is_colored = has_colors ();

	if (is_colored) {
		start_color ();
		init_pair (COLOR_COMMON, COLOR_WHITE, COLOR_BLACK);
		init_pair (COLOR_STEP_DEBUG, COLOR_WHITE, COLOR_YELLOW);
	}

	WINDOW *screen_win;
	WINDOW *hex_editor_win;
	WINDOW *memory_stack_win;
	WINDOW *cpu_win;

	uint32_t width_term = COLS;
	uint32_t height_term = LINES;

	uint32_t real_window_width_game = MAX_WINDOW_GAME_WIDTH + 4;
	uint32_t real_window_height_game = MAX_WINDOW_GAME_HEIGHT + 2;

	memory_stack_win = game_create_window (
			DEBUG_WINDOW_WIDTH,
			DEBUG_WINDOW_HEIGHT,
			0,
			0);

	mvwprintw (memory_stack_win, 0, 1, "stack");

	wrefresh (memory_stack_win);

	cpu_win = game_create_window (
			DEBUG_WINDOW_WIDTH, 
			DEBUG_WINDOW_HEIGHT, 
			width_term - DEBUG_WINDOW_WIDTH,
			0);

	mvwprintw (cpu_win, 0, 1, "cpu");

	wrefresh (cpu_win);

	debug_set_windows (cpu_win, memory_stack_win);

	screen_win = game_create_window (
			real_window_width_game, 
			real_window_height_game, 
			width_term / 2 - real_window_width_game / 2, 0);

	uint32_t hex_editor_width_with_border = width_term;
	uint32_t hex_editor_height_with_border = height_term - real_window_height_game;

	hex_editor_win = game_create_window (
			hex_editor_width_with_border,
			hex_editor_height_with_border, 
			0, 
			real_window_height_game);

	keypad (hex_editor_win, TRUE);


	wrefresh (screen_win);

	hex_editor = hex_editor_create (argv[1], hex_editor_win, 
			hex_editor_width_with_border,
			hex_editor_height_with_border,
			1,
			2);

	hex_editor_draw (hex_editor);

	machine = machine_init (screen_win);
	machine->hex_editor = hex_editor;

	struct timespec editor = {0, 400 * 1000};

	mvwprintw (hex_editor_win, 0, 1, "hex editor");
	mvwprintw (hex_editor_win, 0, 16, " mode: movement ------");
	wrefresh (hex_editor_win);

	mvwprintw (screen_win, 0, 1, "screen");
	wrefresh (screen_win);

	while (1) {
		if (hex_editor->is_quit)
			break;


		if (hex_editor->is_simulate == 0 && !hex_editor->is_debug) {
			hex_editor_draw (hex_editor);
			int c = wgetch (hex_editor->win);
			hex_editor_input (hex_editor, c);
			continue;
		} else {
			if (hex_editor->is_debug) {
				machine_run (machine);
				nanosleep (&machine->timer, NULL);
				continue;
			}
			if (machine->is_run == 0) {
				if (!hex_editor->is_debug) {
					wclear (screen_win);
					box (screen_win, '|', '-');
					mvwprintw (screen_win, 0, 1, "screen");
					wrefresh (screen_win);
					machine->is_run = 1;
					if (!hex_editor->is_debug) {
						machine->cpu.ip = 0x0;
					}
					hex_editor->is_simulate = 1;
				}
			}
			machine_run (machine);
			if (machine->is_run == 0) {
				hex_editor->is_simulate = 0;
				if (!hex_editor->is_debug) {
					machine->cpu.ip = 0x0;
				}
			}
			nanosleep (&machine->timer, NULL);
		}
	}


	endwin ();
	exit (EXIT_SUCCESS);
}
