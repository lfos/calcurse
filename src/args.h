/*	$calcurse: args.h,v 1.1 2006/07/31 21:00:03 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2006 Frederic Culot
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

#ifndef CALCURSE_ARGS_H
#define CALCURSE_ARGS_H

void usage();
void usage_try();
int parse_args(int argc, char **argv, int colr);
void version_arg();
void help_arg();
void todo_arg(int colr);
int app_arg(int add_line, int year, int month, int day, long date);
void date_arg(char *ddate, int add_line);
int check_date(char *date);
void arg_print_date(long date);

#endif /* CALCURSE_ARGS_H */
