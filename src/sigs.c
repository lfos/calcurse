/*	$calcurse: sigs.c,v 1.8 2009/07/05 20:33:23 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2007-2009 Frederic Culot <frederic@culot.org>
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

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "i18n.h"
#include "utils.h"

/* 
 * General signal handling routine.
 * Catch return values from children (user-defined notification commands).
 * This is needed to avoid zombie processes running on system.
 * Also catch CTRL-C (SIGINT), and SIGWINCH to resize screen automatically.
 */
static void
signal_handler (int sig)
{
  switch (sig)
    {
    case SIGCHLD:
      while (waitpid (WAIT_MYPGRP, NULL, WNOHANG) > 0)
	;
      break;
    case SIGWINCH:
      clearok (curscr, TRUE);
      (void)ungetch (KEY_RESIZE);
      break;
    }
}

/* Signal handling init. */
void
sigs_init (struct sigaction *sa)
{
  sa->sa_handler = signal_handler;
  sa->sa_flags = 0;
  sigemptyset (&sa->sa_mask);
  if (sigaction (SIGCHLD, sa, NULL) != 0)
    {
      ERROR_MSG (_("Error handling SIGCHLD signal"));
      exit_calcurse (1);
    }

  sa->sa_handler = signal_handler;
  sa->sa_flags = 0;
  sigemptyset (&sa->sa_mask);
  if (sigaction (SIGWINCH, sa, NULL) != 0)
    {
      ERROR_MSG (_("Error handling SIGWINCH signal"));      
      exit_calcurse (1);
    }

  sa->sa_handler = SIG_IGN;
  sa->sa_flags = 0;
  sigemptyset (&(sa->sa_mask));
  if (sigaction (SIGINT, sa, NULL) != 0)
    {
      ERROR_MSG (_("Error handling SIGINT signal"));      
      exit_calcurse (1);
    }
}
