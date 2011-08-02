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

#include <unistd.h>

#include "calcurse.h"
#include "sha1.h"

/* Edit a note with an external editor. */
void
edit_note (char **note, char *editor)
{
  char tmppath[BUFSIZ];
  char *tmpext;
  char notepath[BUFSIZ];
  char *sha1 = mem_malloc (SHA1_DIGESTLEN * 2 + 1);
  FILE *fp;

  strncpy (tmppath, get_tempdir (), BUFSIZ);
  strncat (tmppath, "/calcurse-note.", BUFSIZ);
  if ((tmpext = new_tempfile (tmppath, TMPEXTSIZ)) == NULL)
    return;
  strncat (tmppath, tmpext, BUFSIZ);
  mem_free (tmpext);

  if (*note != NULL)
    {
      snprintf (notepath, BUFSIZ, "%s%s", path_notes, *note);
      io_file_cp (notepath, tmppath);
    }

  wins_launch_external (tmppath, editor);

  if (io_file_is_empty (tmppath) > 0)
    erase_note (note, ERASE_FORCE_KEEP_NOTE);
  else if ((fp = fopen (tmppath, "r")))
    {
      sha1_stream (fp, sha1);
      fclose (fp);
      *note = sha1;

      snprintf (notepath, BUFSIZ, "%s%s", path_notes, *note);
      io_file_cp (tmppath, notepath);
    }

  unlink (tmppath);
}

/* View a note in an external pager. */
void
view_note (char *note, char *pager)
{
  char fullname[BUFSIZ];

  if (note == NULL)
    return;
  (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, note);
  wins_launch_external (fullname, pager);
}

/* Erase a note previously attached to an item. */
void
erase_note (char **note, enum eraseflg flag)
{
  char fullname[BUFSIZ];

  if (*note == NULL)
    return;
  if (flag != ERASE_FORCE_KEEP_NOTE)
    {
      (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, *note);
      if (unlink (fullname) != 0)
        EXIT (_("could not remove note"));
    }
  mem_free (*note);
  *note = NULL;
}

/* Read a serialized note file name from a stream and deserialize it. */
void
note_read (char *buffer, FILE *fp)
{
  int i;

  for (i = 0; i < MAX_NOTESIZ; i++)
    {
      buffer[i] = getc (fp);
      if (buffer[i] == ' ')
        {
          buffer[i] = '\0';
          return;
        }
    }

  while (getc (fp) != ' ');
  buffer[MAX_NOTESIZ] = '\0';
}
