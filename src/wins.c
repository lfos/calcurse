/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "calcurse.h"

#define SCREEN_ACQUIRE \
  pthread_cleanup_push(screen_cleanup, (void *)NULL); \
  screen_acquire();

#define SCREEN_RELEASE \
  screen_release(); \
  pthread_cleanup_pop(0);

/* Variables to handle calcurse windows. */
struct window win[NBWINS];
struct scrollwin sw_cal;
struct listbox lb_apt;
struct listbox lb_todo;

/* User-configurable side bar width. */
static unsigned sbarwidth_perc;

static enum win slctd_win;
static int layout;

/*
 * The screen_mutex mutex and wins_refresh(), wins_wrefresh(), wins_doupdate()
 * functions are used to prevent concurrent updates of the screen.
 * It was observed that the display could get screwed up when mulitple threads
 * tried to refresh the screen at the same time.
 *
 * Note (2010-03-21):
 * Since recent versions of ncurses (5.7), rudimentary support for threads are
 * available (use_screen(), use_window() for instance - see curs_threads(3)),
 * but to remain compatible with earlier versions, it was chosen to rely on a
 * screen-level mutex instead.
 */
static pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned screen_acquire(void)
{
	if (pthread_mutex_lock(&screen_mutex) != 0)
		return 0;
	else
		return 1;
}

static void screen_release(void)
{
	pthread_mutex_unlock(&screen_mutex);
}

static void screen_cleanup(void *arg)
{
	screen_release();
}

/*
 * FIXME: The following functions currently lock the whole screen. Use both
 * window-level and screen-level mutexes (or use use_screen() and use_window(),
 * see curs_threads(3)) to avoid locking too much.
 */

unsigned wins_nbar_lock(void)
{
	return screen_acquire();
}

void wins_nbar_unlock(void)
{
	screen_release();
}

void wins_nbar_cleanup(void *arg)
{
	wins_nbar_unlock();
}

unsigned wins_calendar_lock(void)
{
	return screen_acquire();
}

void wins_calendar_unlock(void)
{
	screen_release();
}

void wins_calendar_cleanup(void *arg)
{
	wins_calendar_unlock();
}

int wins_refresh(void)
{
	int rc;

	SCREEN_ACQUIRE;
	rc = refresh();
	SCREEN_RELEASE;

	return rc;
}

int wins_wrefresh(WINDOW * win)
{
	int rc;

	if (!win)
		return ERR;
	SCREEN_ACQUIRE;
	rc = wrefresh(win);
	SCREEN_RELEASE;

	return rc;
}

int wins_doupdate(void)
{
	int rc;

	SCREEN_ACQUIRE;
	rc = doupdate();
	SCREEN_RELEASE;

	return rc;
}

/* Get the current layout. */
int wins_layout(void)
{
	return layout;
}

/* Set the current layout. */
void wins_set_layout(int nb)
{
	layout = nb;
}

/* Get the current side bar width. */
unsigned wins_sbar_width(void)
{
	if (sbarwidth_perc > SBARMAXWIDTHPERC) {
		return col * SBARMAXWIDTHPERC / 100;
	} else {
		unsigned sbarwidth =
		    (unsigned)(col * sbarwidth_perc / 100);
		return (sbarwidth <
			SBARMINWIDTH) ? SBARMINWIDTH : sbarwidth;
	}
}

/*
 * Return the side bar width in percentage of the total number of columns
 * available in calcurse's screen.
 */
unsigned wins_sbar_wperc(void)
{
	return sbarwidth_perc >
	    SBARMAXWIDTHPERC ? SBARMAXWIDTHPERC : sbarwidth_perc;
}

/*
 * Set side bar width (unit: number of characters) given a width in percentage
 * of calcurse's screen total width.
 * The side bar could not have a width representing more than 50% of the screen,
 * and could not be less than SBARMINWIDTH characters.
 */
void wins_set_sbar_width(unsigned perc)
{
	sbarwidth_perc = perc;
}

/* Change the width of the side bar within acceptable boundaries. */
void wins_sbar_winc(void)
{
	if (sbarwidth_perc < SBARMAXWIDTHPERC)
		sbarwidth_perc++;
}

void wins_sbar_wdec(void)
{
	if (sbarwidth_perc > 0)
		sbarwidth_perc--;
}

/* Returns an enum which corresponds to the window which is selected. */
enum win wins_slctd(void)
{
	return slctd_win;
}

/* Sets the selected window. */
void wins_slctd_set(enum win window)
{
	slctd_win = window;
}

/* TAB key was hit in the interface, need to select next window. */
void wins_slctd_next(void)
{
	if (slctd_win == TOD)
		slctd_win = CAL;
	else
		slctd_win++;
}

static void wins_init_panels(void)
{
	wins_scrollwin_init(&sw_cal, win[CAL].y, win[CAL].x,
			    CALHEIGHT + (conf.compact_panels ? 2 : 4),
			    wins_sbar_width(), _("Calendar"));

	listbox_init(&lb_apt, win[APP].y, win[APP].x, win[APP].h, win[APP].w,
		     _("Appointments"), ui_day_row_type, ui_day_height,
		     ui_day_draw);
	ui_day_load_items();

	listbox_init(&lb_todo, win[TOD].y, win[TOD].x, win[TOD].h, win[TOD].w,
		     _("TODO"), ui_todo_row_type, ui_todo_height,
		     ui_todo_draw);
	ui_todo_load_items();
}

/* Create all the windows. */
void wins_init(void)
{
	wins_init_panels();
	win[STA].p =
	    newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
	win[KEY].p = newwin(1, 1, 1, 1);

	keypad(win[STA].p, TRUE);
	keypad(win[KEY].p, TRUE);

	/* Notify that the curses mode is now launched. */
	ui_mode = UI_CURSES;
}

/*
 * Create a new window and its associated pad, which is used to make the
 * scrolling faster.
 */
void wins_scrollwin_init(struct scrollwin *sw, int y, int x, int h, int w, const char *label)
{
	EXIT_IF(sw == NULL, "null pointer");
	sw->y = y;
	sw->x = x;
	sw->h = h;
	sw->w = w;
	sw->win = newwin(h, w, y, x);
	sw->inner = newpad(BUFSIZ, w);
	sw->line_num = sw->line_off = 0;
	sw->label = label;
}

/* Resize a scrolling window. */
void wins_scrollwin_resize(struct scrollwin *sw, int y, int x, int h, int w)
{
	EXIT_IF(sw == NULL, "null pointer");
	sw->y = y;
	sw->x = x;
	sw->h = h;
	sw->w = w;
	delwin(sw->inner);
	delwin(sw->win);
	sw->win = newwin(h, w, y, x);
	sw->inner = newpad(BUFSIZ, w);
}

/*
 * Set the number of lines to be displayed.
 */
void wins_scrollwin_set_linecount(struct scrollwin *sw, unsigned lines)
{
	sw->line_num = lines;
}

/* Free an already created scrollwin. */
void wins_scrollwin_delete(struct scrollwin *sw)
{
	EXIT_IF(sw == NULL, "null pointer");
	delwin(sw->inner);
	delwin(sw->win);
}

/* Draw window border and label. */
void wins_scrollwin_draw_deco(struct scrollwin *sw, int hilt)
{
	if (hilt)
		wattron(sw->win, A_BOLD | COLOR_PAIR(COLR_CUSTOM));

	box(sw->win, 0, 0);

	if (!conf.compact_panels) {
		mvwaddch(sw->win, 2, 0, ACS_LTEE);
		mvwhline(sw->win, 2, 1, ACS_HLINE, sw->w - 2);
		mvwaddch(sw->win, 2, sw->w - 1, ACS_RTEE);
	}

	if (hilt)
		wattroff(sw->win, A_BOLD | COLOR_PAIR(COLR_CUSTOM));

	if (!conf.compact_panels)
		print_in_middle(sw->win, 1, 0, sw->w, sw->label);
}

/* Display a scrolling window. */
void wins_scrollwin_display(struct scrollwin *sw)
{
	int inner_y = (conf.compact_panels ? 1 : 3);
	int inner_x = 1;
	int inner_h = sw->h - (conf.compact_panels ? 2 : 4);
	int inner_w = sw->w - 2;

	if (sw->line_num > inner_h) {
		int sbar_h = MAX(inner_h * inner_h / sw->line_num, 1);
		int sbar_y = inner_y + sw->line_off * (inner_h - sbar_h) / (sw->line_num - inner_h);
		int sbar_x = sw->w - 1;

		draw_scrollbar(sw->win, sbar_y, sbar_x, sbar_h, inner_y,
			       inner_y + inner_h - 1, 1);
	}

	wmove(win[STA].p, 0, 0);
	wnoutrefresh(sw->win);
	pnoutrefresh(sw->inner, sw->line_off, 0, sw->y + inner_y,
		     sw->x + inner_x, sw->y + inner_y + inner_h - 1,
		     sw->x + inner_x + inner_w - 1);
	wins_doupdate();
}

void wins_scrollwin_up(struct scrollwin *sw, int amount)
{
	if ((int)sw->line_off - amount > 0)
		sw->line_off -= amount;
	else
		sw->line_off = 0;
}

void wins_scrollwin_down(struct scrollwin *sw, int amount)
{
	int inner_h = sw->h - (conf.compact_panels ? 2 : 4);

	sw->line_off += amount;

	if ((int)sw->line_off > (int)sw->line_num - inner_h)
		sw->line_off = sw->line_num - inner_h;
}

int wins_scrollwin_is_visible(struct scrollwin *sw, unsigned line)
{
	int inner_h = sw->h - (conf.compact_panels ? 2 : 4);
	return ((line >= sw->line_off) && (line < sw->line_off + inner_h));
}

void wins_scrollwin_ensure_visible(struct scrollwin *sw, unsigned line)
{
	int inner_h = sw->h - (conf.compact_panels ? 2 : 4);

	if (line < sw->line_off)
		sw->line_off = line;
	else if (line >= sw->line_off + inner_h)
		sw->line_off = line - inner_h + 1;
}

void wins_scrollwin_set_lower(struct scrollwin *sw, unsigned line)
{
	int inner_h = sw->h - (conf.compact_panels ? 2 : 4);
	sw->line_off = MAX((int)line - inner_h + 1, 0);
}

void wins_resize_panels(void)
{
	wins_get_config();
	wins_scrollwin_resize(&sw_cal, win[CAL].y, win[CAL].x,
			      CALHEIGHT + (conf.compact_panels ? 2 : 4),
			      wins_sbar_width());
	listbox_resize(&lb_apt, win[APP].y, win[APP].x, win[APP].h,
		       win[APP].w);
	listbox_resize(&lb_todo, win[TOD].y, win[TOD].x, win[TOD].h,
		       win[TOD].w);
}

/*
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
void wins_resize(void)
{
	wins_resize_panels();

	delwin(win[STA].p);
	delwin(win[KEY].p);
	win[STA].p = newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
	win[KEY].p = newwin(1, 1, 1, 1);

	keypad(win[STA].p, TRUE);
	keypad(win[KEY].p, TRUE);

	if (notify_bar())
		notify_reinit_bar();
}

/* Show the window with a border and a label. */
void wins_show(WINDOW * win, const char *label)
{
	int width = getmaxx(win);

	box(win, 0, 0);

	if (!conf.compact_panels) {
		mvwaddch(win, 2, 0, ACS_LTEE);
		mvwhline(win, 2, 1, ACS_HLINE, width - 2);
		mvwaddch(win, 2, width - 1, ACS_RTEE);

		print_in_middle(win, 1, 0, width, label);
	}
}

/*
 * Get the screen size and recalculate the windows configurations.
 */
void wins_get_config(void)
{
	enum win win_master;
	enum win win_slave[2];
	unsigned master_is_left;

	/* Get the screen configuration */
	getmaxyx(stdscr, row, col);

	/* fixed values for status, notification bars and calendar */
	win[STA].h = STATUSHEIGHT;
	win[STA].w = col;
	win[STA].y = row - win[STA].h;
	win[STA].x = 0;

	if (notify_bar()) {
		win[NOT].h = 1;
		win[NOT].w = col;
		win[NOT].y = win[STA].y - 1;
		win[NOT].x = 0;
	} else {
		win[NOT].h = 0;
		win[NOT].w = 0;
		win[NOT].y = 0;
		win[NOT].x = 0;
	}

	win[CAL].h = CALHEIGHT + (conf.compact_panels ? 2 : 4);

	if (layout <= 4) {
		win_master = APP;
		win_slave[0] = ((layout - 1) % 2 == 0) ? CAL : TOD;
		win_slave[1] = ((layout - 1) % 2 == 1) ? CAL : TOD;
		win[TOD].h = row - (win[CAL].h + win[STA].h + win[NOT].h);
	} else {
		win_master = TOD;
		win_slave[0] = ((layout - 1) % 2 == 0) ? CAL : APP;
		win_slave[1] = ((layout - 1) % 2 == 1) ? CAL : APP;
		win[APP].h = row - (win[CAL].h + win[STA].h + win[NOT].h);
	}
	master_is_left = ((layout - 1) % 4 < 2);

	win[win_master].x = master_is_left ? 0 : wins_sbar_width();
	win[win_master].y = 0;
	win[win_master].w = col - wins_sbar_width();
	win[win_master].h = row - (win[STA].h + win[NOT].h);

	win[win_slave[0]].x = win[win_slave[1]].x =
	    master_is_left ? win[win_master].w : 0;
	win[win_slave[0]].y = 0;
	win[win_slave[1]].y = win[win_slave[0]].h;
	win[win_slave[0]].w = win[win_slave[1]].w = wins_sbar_width();
}

void wins_update_border(int flags)
{
	if (flags & FLAG_CAL) {
		WINS_CALENDAR_LOCK;
		wins_scrollwin_draw_deco(&sw_cal, (slctd_win == CAL));
		WINS_CALENDAR_UNLOCK;
	}
	if (flags & FLAG_APP)
		listbox_draw_deco(&lb_apt, (slctd_win == APP));
	if (flags & FLAG_TOD)
		listbox_draw_deco(&lb_todo, (slctd_win == TOD));
}

void wins_update_panels(int flags)
{
	if (flags & FLAG_APP)
		ui_day_update_panel(slctd_win);
	if (flags & FLAG_TOD)
		ui_todo_update_panel(slctd_win);
	if (flags & FLAG_CAL)
		ui_calendar_update_panel();
}

/*
 * Update all of the three windows and put a border around the
 * selected window.
 */
void wins_update(int flags)
{
	wins_update_border(flags);
	wins_update_panels(flags);
	if (flags & FLAG_STA) {
		wins_update_bindings();
		wins_status_bar();
	}
	if ((flags & FLAG_NOT) && notify_bar())
		notify_update_bar();
	wmove(win[STA].p, 0, 0);
	wins_doupdate();
}

/* Reset the screen, needed when resizing terminal for example. */
void wins_reset_noupdate(void)
{
	endwin();
	wins_refresh();
	curs_set(0);
	wins_resize();
}

/* Reset the screen, needed when resizing terminal for example. */
void wins_reset(void)
{
	wins_reset_noupdate();
	wins_update(FLAG_ALL);
}

/* Prepare windows for the execution of an external command. */
void wins_prepare_external(void)
{
	if (notify_bar())
		notify_stop_main_thread();
	def_prog_mode();
	ui_mode = UI_CMDLINE;
	clear();
	wins_refresh();
	endwin();
	sigs_ignore();
}

/* Restore windows when returning from an external command. */
void wins_unprepare_external(void)
{
	sigs_unignore();
	reset_prog_mode();
	clearok(curscr, TRUE);
	curs_set(0);
	ui_mode = UI_CURSES;
	wins_refresh();
	wins_resize();
	wins_update(FLAG_ALL);
	if (notify_bar())
		notify_start_main_thread();
}

/*
 * While inside interactive mode, launch the external command cmd on the given
 * file.
 */
void wins_launch_external(const char *arg[])
{
	int pid;

	wins_prepare_external();
	if ((pid = shell_exec(NULL, NULL, *arg, arg)))
		child_wait(NULL, NULL, pid);
	wins_unprepare_external();
}

static int *bindings;
static int bindings_size;
static unsigned status_page;

/* Sets the current set of key bindings to display in the status bar. */
void wins_set_bindings(int *new_bindings, int size)
{
	bindings = new_bindings;
	bindings_size = size;
}

/*
 * Obtains the set of key bindings to display for the active panel.
 *
 * To add a key binding, insert a new binding_t item and add it to the binding
 * table.
 */
void wins_update_bindings(void)
{
	static int bindings_cal[] = {
		KEY_GENERIC_HELP, KEY_GENERIC_QUIT, KEY_GENERIC_SAVE,
		KEY_GENERIC_RELOAD, KEY_GENERIC_CHANGE_VIEW,
		KEY_GENERIC_SCROLL_DOWN, KEY_GENERIC_SCROLL_UP, KEY_MOVE_UP,
		KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_GENERIC_GOTO,
		KEY_GENERIC_IMPORT, KEY_GENERIC_EXPORT, KEY_START_OF_WEEK,
		KEY_END_OF_WEEK, KEY_GENERIC_ADD_APPT, KEY_GENERIC_ADD_TODO,
		KEY_GENERIC_PREV_DAY, KEY_GENERIC_NEXT_DAY,
		KEY_GENERIC_PREV_WEEK, KEY_GENERIC_NEXT_WEEK,
		KEY_GENERIC_PREV_MONTH, KEY_GENERIC_NEXT_MONTH,
		KEY_GENERIC_PREV_YEAR, KEY_GENERIC_NEXT_YEAR,
		KEY_GENERIC_REDRAW, KEY_GENERIC_GOTO_TODAY,
		KEY_GENERIC_CONFIG_MENU, KEY_GENERIC_CMD
	};

	static int bindings_apoint[] = {
		KEY_GENERIC_HELP, KEY_GENERIC_QUIT, KEY_GENERIC_SAVE,
		KEY_GENERIC_RELOAD, KEY_GENERIC_CHANGE_VIEW,
		KEY_GENERIC_IMPORT, KEY_GENERIC_EXPORT, KEY_ADD_ITEM,
		KEY_DEL_ITEM, KEY_EDIT_ITEM, KEY_VIEW_ITEM, KEY_PIPE_ITEM,
		KEY_GENERIC_REDRAW, KEY_REPEAT_ITEM, KEY_FLAG_ITEM,
		KEY_EDIT_NOTE, KEY_VIEW_NOTE, KEY_MOVE_UP, KEY_MOVE_DOWN,
		KEY_GENERIC_PREV_DAY, KEY_GENERIC_NEXT_DAY,
		KEY_GENERIC_PREV_WEEK, KEY_GENERIC_NEXT_WEEK,
		KEY_GENERIC_PREV_MONTH, KEY_GENERIC_NEXT_MONTH,
		KEY_GENERIC_PREV_YEAR, KEY_GENERIC_NEXT_YEAR, KEY_GENERIC_GOTO,
		KEY_GENERIC_GOTO_TODAY, KEY_GENERIC_CONFIG_MENU,
		KEY_GENERIC_ADD_APPT, KEY_GENERIC_ADD_TODO, KEY_GENERIC_COPY,
		KEY_GENERIC_PASTE, KEY_GENERIC_CMD
	};

	static int bindings_todo[] = {
		KEY_GENERIC_HELP, KEY_GENERIC_QUIT, KEY_GENERIC_SAVE,
		KEY_GENERIC_RELOAD, KEY_GENERIC_CHANGE_VIEW,
		KEY_GENERIC_SCROLL_DOWN, KEY_GENERIC_SCROLL_UP,
		KEY_GENERIC_IMPORT, KEY_GENERIC_EXPORT, KEY_ADD_ITEM,
		KEY_DEL_ITEM, KEY_EDIT_ITEM, KEY_VIEW_ITEM, KEY_PIPE_ITEM,
		KEY_FLAG_ITEM, KEY_RAISE_PRIORITY, KEY_LOWER_PRIORITY,
		KEY_EDIT_NOTE, KEY_VIEW_NOTE, KEY_MOVE_UP, KEY_MOVE_DOWN,
		KEY_GENERIC_PREV_DAY, KEY_GENERIC_NEXT_DAY,
		KEY_GENERIC_PREV_WEEK, KEY_GENERIC_NEXT_WEEK,
		KEY_GENERIC_PREV_MONTH, KEY_GENERIC_NEXT_MONTH,
		KEY_GENERIC_PREV_YEAR, KEY_GENERIC_NEXT_YEAR, KEY_GENERIC_GOTO,
		KEY_GENERIC_GOTO_TODAY, KEY_GENERIC_CONFIG_MENU,
		KEY_GENERIC_ADD_APPT, KEY_GENERIC_ADD_TODO, KEY_GENERIC_REDRAW,
		KEY_GENERIC_CMD
	};

	enum win active_panel = wins_slctd();

	switch (active_panel) {
	case CAL:
		wins_set_bindings(bindings_cal, ARRAY_SIZE(bindings_cal));
		break;
	case APP:
		wins_set_bindings(bindings_apoint,
				ARRAY_SIZE(bindings_apoint));
		break;
	case TOD:
		wins_set_bindings(bindings_todo, ARRAY_SIZE(bindings_todo));
		break;
	default:
		EXIT(_("unknown panel"));
		/* NOTREACHED */
	}
}

/* Draws the status bar. */
void wins_status_bar(void)
{
	int page_base = (KEYS_CMDS_PER_LINE * 2 - 1) * (status_page - 1);
	int page_size = KEYS_CMDS_PER_LINE * 2;

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size,
				  page_base, page_size);
}

/* Erase status bar. */
void wins_erase_status_bar(void)
{
	erase_window_part(win[STA].p, 0, 0, col, STATUSHEIGHT);
}

/* Update the status bar page number to display other commands. */
void wins_other_status_page(int panel)
{
	/*
	 * Determine the number of completely filled pages of key bindings.
	 * There are two lines of bindings and KEYS_CMDS_PER_LINE bindings per
	 * line. On each page (other than the last page), one slot is reserved
	 * for OtherCmd.
	 */
	const int slots_per_page = KEYS_CMDS_PER_LINE * 2 - 1;
	int max_page = bindings_size / slots_per_page;

	/*
	 * The result of the previous computation might have been rounded down.
	 * In this case, there are some bindings left. If there is exactly one
	 * binding left, it can be squashed onto the last page in place of the
	 * OtherCmd binding. If there are at least two bindings left, we need
	 * to add another page.
	 */
	if (bindings_size % slots_per_page > 1)
		max_page++;

	status_page = (status_page % max_page) + 1;
}

/* Reset the status bar page. */
void wins_reset_status_page(void)
{
	status_page = 1;
}
