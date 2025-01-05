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
#include <time.h>

static struct hex_editor *hex_editor;
static struct machine *machine;

static WINDOW *
game_create_window (int w, int h, int x, int y)
{
	WINDOW *win = newwin (h, w, y, x);
	box (win, '|', '-');
	return win;
}

static void *
handle_input (void *_data)
{
	WINDOW *win = _data;
	while (1) {
		int c = wgetch (win);

		if (!hex_editor->is_simulate) {
			hex_editor_input (hex_editor, c);
			if (hex_editor->is_quit) {
				return NULL;
			}
		} else {
			machine_input (machine, c);
		}
	}
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
	keypad (stdscr, TRUE);
	cbreak ();
	noecho ();

	WINDOW *screen_win;
	WINDOW *hex_editor_win;

	uint32_t width_term = COLS;
	uint32_t height_term = LINES;

	uint32_t real_window_width_game = MAX_WINDOW_GAME_WIDTH + 4;
	uint32_t real_window_height_game = MAX_WINDOW_GAME_HEIGHT + 2;

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

	wrefresh (screen_win);

	hex_editor = hex_editor_create (argv[1], hex_editor_win, 
			hex_editor_width_with_border,
			hex_editor_height_with_border,
			1,
			2);

	hex_editor_draw (hex_editor);

	machine = machine_init (screen_win);
	machine->hex_editor = hex_editor;

	pthread_t p_handle_input;
	pthread_create (&p_handle_input, NULL, handle_input, hex_editor_win);

	struct timespec editor = {0, 400 * 1000};

	while (1) {
		if (hex_editor->is_quit)
			break;

		if (hex_editor->is_simulate == 0) {
			hex_editor_draw (hex_editor);
			nanosleep (&editor, NULL);
		} else {
			if (machine->is_run == 0) {
				wclear (screen_win);
				box (screen_win, '|', '-');
				wrefresh (screen_win);
				machine->is_run = 1;
				machine->cpu.ip = 0x0;
				hex_editor->is_simulate = 1;
			}
			machine_run (machine);
			if (machine->is_run == 0) {
				hex_editor->is_simulate = 0;
				machine->cpu.ip = 0x0;
			}
			nanosleep (&machine->timer, NULL);
		}

	}


	endwin ();
	exit (EXIT_SUCCESS);
}
