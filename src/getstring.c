/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2011 calcurse Development Team <misc@calcurse.org>
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

#include "calcurse.h"

/* Print the string at the desired position. */
static void
showstring (WINDOW *win, int x, int y, char *str, int len, int scroff,
            int curpos)
{
  char c = 0;

  /* print string */
  mvwaddnstr (win, y, x, &str[scroff], -1);
  wclrtoeol (win);

  /* print scrolling indicator */
  if (scroff > 0 && scroff < len - col)
    c = '*';
  else if (scroff > 0)
    c = '<';
  else if (scroff < len - col)
    c = '>';
  mvwprintw (win, y, col - 1, "%c", c);

  /* print cursor */
  wmove (win, y, curpos - scroff);

  if (curpos >= len)
    waddch (win, SPACE | A_REVERSE);
  else
    waddch (win, str[curpos] | A_REVERSE);
}

/* Delete a character at the given position in string. */
static void
del_char (int pos, char *str)
{
  str += pos;
  memmove (str, str + 1, strlen (str) + 1);
}

/* Add a character at the given position in string. */
static void
ins_char (int pos, int ch, char *str)
{
  str += pos;
  memmove (str + 1, str, strlen (str) + 1);
  *str = ch;
}

static void
bell (void)
{
  printf ("\a");
}

/*
 * Getstring allows to get user input and to print it on a window,
 * even if noecho() is on. This function is also used to modify an existing
 * text (the variable string can be non-NULL).
 * We need to do the echoing manually because of the multi-threading
 * environment, otherwise the cursor would move from place to place without
 * control.
 */
enum getstr
getstring (WINDOW *win, char *str, int l, int x, int y)
{
  const int pgsize = col / 3;

  int len = strlen (str);
  int curpos = len;
  int scroff = 0;
  int ch;

  custom_apply_attr (win, ATTR_HIGHEST);

  for (;;) {
    while (curpos < scroff)
      scroff -= pgsize;
    while (curpos >= scroff + col - 1)
      scroff += pgsize;

    showstring (win, x, y, str, len, scroff, curpos);
    wins_doupdate ();

    if ((ch = wgetch (win)) == '\n') break;
    switch (ch)
      {
        case KEY_BACKSPACE:     /* delete one character */
        case 330:
        case 127:
        case CTRL ('H'):
          if (curpos > 0)
            {
              del_char ((--curpos), str);
              len--;
            }
          else
            bell ();
          break;
        case CTRL ('D'):        /* delete next character */
          if (curpos < len)
            {
              del_char (curpos, str);
              len--;
            }
          else
            bell ();
          break;
        case CTRL ('W'):        /* delete a word */
          if (curpos > 0) {
            while (curpos && str[curpos - 1] == ' ')
              {
                del_char ((--curpos), str);
                len--;
              }
            while (curpos && str[curpos - 1] != ' ')
              {
                del_char ((--curpos), str);
                len--;
              }
          }
          else
            bell ();
          break;
        case CTRL ('K'):        /* delete to end-of-line */
          str[curpos] = 0;
          len = curpos;
          break;
        case CTRL ('A'):        /* go to begginning of string */
          curpos = 0;
          break;
        case CTRL ('E'):        /* go to end of string */
          curpos = len;
          break;
        case KEY_LEFT:          /* move one char backward  */
        case CTRL ('B'):
          if (curpos > 0) curpos--;
          break;
        case KEY_RIGHT:         /* move one char forward */
        case CTRL ('F'):
          if (curpos < len) curpos++;
          break;
        case ESCAPE:            /* cancel editing */
          return (GETSTRING_ESC);
          break;
        default:                /* insert one character */
          if (len < l - 1)
            {
              ins_char ((curpos++), ch, str);
              len++;
            }
      }
  }

  custom_remove_attr (win, ATTR_HIGHEST);

  return (len == 0 ? GETSTRING_RET : GETSTRING_VALID);
}

/* Update an already existing string. */
int
updatestring (WINDOW *win, char **str, int x, int y)
{
  int len = strlen (*str);
  char *buf;
  enum getstr ret;

  EXIT_IF (len + 1 > BUFSIZ, _("Internal error: line too long"));

  buf = mem_malloc (BUFSIZ);
  (void)memcpy (buf, *str, len + 1);

  ret = getstring (win, buf, BUFSIZ, x, y);

  if (ret == GETSTRING_VALID)
    {
      len = strlen (buf);
      *str = mem_realloc (*str, len + 1, 1);
      EXIT_IF (*str == NULL, _("out of memory"));
      (void)memcpy (*str, buf, len + 1);
    }

  mem_free (buf);
  return ret;
}
