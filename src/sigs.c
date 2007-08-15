/*	$Id: sigs.c,v 1.3 2007/08/15 15:30:17 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2007 Frederic Culot
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

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#include "i18n.h"
#include "vars.h"
#include "wins.h"

/* 
 * General signal handling routine.
 * Catch return values from children (user-defined notification commands).
 * This is needed to avoid zombie processes running on system.
 * Also catch CTRL-C (SIGINT), and SIGWINCH to resize screen automatically.
 */
static void
signal_handler(int sig)
{
	switch (sig) {
	case SIGCHLD:
		while (waitpid(WAIT_MYPGRP, NULL, WNOHANG) > 0)
			;
		break;
	case SIGWINCH:
		wins_reset();
		break;
	}
}

/* Signal handling init. */
void
sigs_init(struct sigaction *sa)
{
	sa->sa_handler = signal_handler;
	sa->sa_flags = 0;
	sigemptyset(&sa->sa_mask);
	if (sigaction(SIGCHLD, sa, NULL) != 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	sa->sa_handler = signal_handler;
	sa->sa_flags = 0;
	sigemptyset(&sa->sa_mask);
	if (sigaction(SIGWINCH, sa, NULL) != 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	sa->sa_handler = SIG_IGN; 
	sa->sa_flags = 0;
	sigemptyset(&(sa->sa_mask));
	if (sigaction(SIGINT, sa, NULL) != 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}
