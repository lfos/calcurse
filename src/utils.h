/*	$calcurse: utils.h,v 1.46 2009/07/12 16:22:02 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2009 Frederic Culot <frederic@culot.org>
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
  char msg[BUFSIZ];                                                     \
  int len;                                                              \
                                                                        \
  len = snprintf (msg, BUFSIZ, "%s: %d: ", __FILE__, __LINE__);         \
  (void)snprintf (msg + len, BUFSIZ - len, __VA_ARGS__);                \
  if (ui_mode == UI_CURSES)                                             \
    fatalbox (msg);                                                     \
  else                                                                  \
    (void)fprintf (stderr, "%s\n", msg);                                \
} while (0)

#define WARN_MSG(...) do {                                              \
  char msg[BUFSIZ];                                                     \
                                                                        \
  (void)snprintf (msg, BUFSIZ, __VA_ARGS__);                            \
  if (ui_mode == UI_CURSES)                                             \
    warnbox (msg);                                                      \
  else                                                                  \
    (void)fprintf (stderr, "%s\n", msg);                                \
} while (0)

#define EXIT(...) do {                                                  \
          ERROR_MSG(__VA_ARGS__);                                       \
          if (ui_mode == UI_CURSES)                                     \
            exit_calcurse (EXIT_FAILURE);                               \
          else                                                          \
            exit (EXIT_FAILURE);                                        \
} while (0)

#define EXIT_IF(cond, ...) do {                                         \
  if ((cond))                                                           \
    EXIT(__VA_ARGS__);                                                  \
} while (0)

#define RETURN_IF(cond, ...) do {                                       \
  if ((cond))                                                           \
    {                                                                   \
      ERROR_MSG(__VA_ARGS__);                                           \
      return;                                                           \
    }                                                                   \
} while (0)

#define RETVAL_IF(cond, val, ...) do {                                  \
  if ((cond))                                                           \
    {                                                                   \
      ERROR_MSG(__VA_ARGS__);                                           \
      return (val);                                                     \
    }                                                                   \
} while (0)

#define STRINGIFY(x)  #x
#define TOSTRING(x)   STRINGIFY(x)
#define __FILE_POS__   __FILE__ ":" TOSTRING(__LINE__)

#define GETSTRING_VALID	0   /* value returned by getstring() if text is valid */
#define GETSTRING_ESC	1   /* user pressed escape to cancel editing */
#define GETSTRING_RET	2   /* return was pressed without entering any text */

typedef struct {
  const char *str;
  const int len;
} string_t;

typedef enum
{
  ERASE_DONT_FORCE,
  ERASE_FORCE,
  ERASE_FORCE_KEEP_NOTE,
  ERASE_FORCE_ONLY_NOTE,
  ERASE_CUT
}
erase_flag_e;

void      exit_calcurse (int);
void      fatalbox (const char *);
void      warnbox (const char *);
void      status_mesg (char *, char *);
void      erase_window_part (WINDOW *, int, int, int, int);
WINDOW   *popup (int, int, int, int, char *, char *, int);
void      print_in_middle (WINDOW *, int, int, int, char *);
int       getstring (WINDOW *, char *, int, int, int);
int       updatestring (WINDOW *, char **, int, int);
int       is_all_digit (char *);
long      get_item_time (long);
int       get_item_hour (long);
int       get_item_min (long);
long      date2sec (date_t, unsigned, unsigned);
char     *date_sec2hour_str (long);
char     *date_sec2date_str (long, char *);
void      date_sec2date_fmt (long, const char *, char *);
long      date_sec_change (long, int, int);
long      update_time_in_date (long, unsigned, unsigned);
long      get_sec_date (date_t);
long      min2sec (unsigned);
int       check_time (char *);
void      draw_scrollbar (WINDOW *, int, int, int, int, int, unsigned);
void      item_in_popup (char *, char *, char *, char *);
long      get_today (void);
long      now (void);
long      mystrtol (const char *);
void      print_bool_option_incolor (WINDOW *, unsigned, int, int);
char     *new_tempfile (const char *, int);
void      erase_note (char **, erase_flag_e);
int       parse_date (char *, int, int *, int *, int *);
char     *str_toupper (char *);
void      file_close (FILE *, const char *);

#endif /* CALCURSE_UTILS_H */
