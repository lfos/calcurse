/*	$calcurse: utils.h,v 1.39 2008/11/23 20:38:56 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#ifndef CALCURSE_UTILS_H
#define CALCURSE_UTILS_H

#include <stdlib.h>

#include "calendar.h"
#include "vars.h"

#define MAX(x,y) 	((x)>(y)?(x):(y))
#define MIN(x,y) 	((x)<(y)?(x):(y))

#define STRING_BUILD(str) {str, sizeof (str) - 1}

#define ERROR_MSG(...) do {                                             \
        char msg[BUFSIZ];                                               \
                                                                        \
        snprintf (msg, BUFSIZ, __VA_ARGS__);                            \
        if (ui_mode == UI_CURSES)                                       \
          warnbox (msg);                                                \
        else                                                            \
          fprintf (stderr, "%s\n", msg);                                \
} while (0)

#define EXIT(...) do {                                                  \
          ERROR_MSG(__VA_ARGS__);                                       \
          if (ui_mode == UI_CURSES)                                     \
            exit_calcurse (EXIT_FAILURE);                               \
          else                                                          \
            exit (EXIT_FAILURE);                                        \
} while (0)

#define EXIT_IF(cond, ...) do {                                         \
        if ((cond))                                                     \
          {                                                             \
            ERROR_MSG(__VA_ARGS__);                                     \
            if (ui_mode == UI_CURSES)                                   \
              exit_calcurse (EXIT_FAILURE);                             \
            else                                                        \
              exit (EXIT_FAILURE);                                      \
          }                                                             \
} while (0)

#define RETURN_IF(cond, ...) do {                                       \
        if ((cond))                                                     \
          {                                                             \
            ERROR_MSG(__VA_ARGS__);                                     \
            return;                                                     \
          }                                                             \
} while (0)

#define RETVAL_IF(cond, val, ...) do {                                  \
        if ((cond))                                                     \
          {                                                             \
            ERROR_MSG(__VA_ARGS__);                                     \
            return (val);                                               \
          }                                                             \
} while (0)

#define ASSERT(e) do {							\
	((e) ? (void)0 : aerror(__FILE__, __LINE__, #e));		\
} while (0)

#define GETSTRING_VALID	0	/* value returned by getstring() if text is valid */
#define GETSTRING_ESC	1	/* user pressed escape to cancel editing */
#define GETSTRING_RET	2	/* return was pressed without entering any text */

typedef struct {
  const char *str;
  const int len;
} string_t;

typedef enum
{
  IERROR_FATAL,
  IERROR_WARN
}
ierror_sev_e;

typedef enum
{
  ERASE_DONT_FORCE,
  ERASE_FORCE,
  ERASE_FORCE_KEEP_NOTE,
  ERASE_FORCE_ONLY_NOTE
}
erase_flag_e;

void    exit_calcurse (int);
void    ierror (const char *, ierror_sev_e);
void    aerror (const char *, int, const char *);
void    warnbox (const char *);
void    status_mesg (char *, char *);
void    erase_status_bar (void);
void    erase_window_part (WINDOW *, int, int, int, int);
WINDOW *popup (int, int, int, int, char *, char *, int);
void    print_in_middle (WINDOW *, int, int, int, char *);
int     getstring (WINDOW *, char *, int, int, int);
int     updatestring (WINDOW *, char **, int, int);
int     is_all_digit (char *);
void    status_bar (void);
long    date2sec (date_t, unsigned, unsigned);
char   *date_sec2hour_str (long);
char   *date_sec2date_str (long, char *);
void    date_sec2date_fmt (long, const char *, char *);
long    date_sec_change (long, int, int);
long    update_time_in_date (long, unsigned, unsigned);
long    get_sec_date (date_t);
long    min2sec (unsigned);
int     check_time (char *);
void    draw_scrollbar (WINDOW *, int, int, int, int, int, bool);
void    item_in_popup (char *, char *, char *, char *);
void    reset_status_page (void);
void    other_status_page (int);
long    get_today (void);
long    now (void);
char   *mycpy (const char *);
long    mystrtol (const char *);
void    print_bool_option_incolor (WINDOW *, bool, int, int);
char   *new_tempfile (const char *, int);
void    erase_note (char **, erase_flag_e);
int     parse_date (char *, int, int *, int *, int *);
char   *str_toupper (char *);
void    mem_free (void *ptr);

#endif /* CALCURSE_UTILS_H */
