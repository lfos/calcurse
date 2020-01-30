/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
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

/*
 * A queue for calcurse system messages.
 */
llist_t sysqueue;
static pthread_mutex_t que_mutex = PTHREAD_MUTEX_INITIALIZER;

void que_init(void)
{
	LLIST_INIT(&sysqueue);
}

/*
 * Test for queued system events.
 */
int que_ued(void)
{
	return sysqueue.head ? 1 : 0;
}

/*
 * Insert a system event at the tail of the queue.
 */
struct event *que_ins(char *mesg, time_t time, int id)
{
	struct event *ev;

	ev = mem_malloc(sizeof(struct event));
	ev->mesg = mem_strdup(mesg);
	ev->day = time;
	ev->id = id;
	ev->note = NULL;
	pthread_mutex_lock(&que_mutex);
	LLIST_ADD(&sysqueue, ev);
	pthread_mutex_unlock(&que_mutex);

	return ev;
}

/*
 * Get the system event at the head of the queue.
 */
struct event *que_get(void)
{
	return sysqueue.head ? sysqueue.head->data : NULL;
}

/*
 * Remove the system event at the head of the queue.
 */
void que_rem(void)
{
	struct event *ev;

	if (!sysqueue.head)
		return;
	ev = sysqueue.head->data;

	pthread_mutex_lock(&que_mutex);
	LLIST_REMOVE(&sysqueue, sysqueue.head);
	pthread_mutex_unlock(&que_mutex);

	mem_free(ev->mesg);
	mem_free(ev);
}

/*
 * Display the system event at the head of the queue in a popup window.
 */
void que_show(void)
{
	struct event *ev;
	char *date;

	if (!que_ued())
		return;
	ev = que_get();
	date = date_sec2date_str(ev->day, "%F %T");
	item_in_popup(date, "", ev->mesg, _("System event"));
	mem_free(date);
}

/*
 * Save the system event at the head of the queue as an appointment.
 */
void que_save(void)
{
	struct event *ev;

	if (!que_ued())
		return;
	ev = que_get();
	apoint_new(ev->mesg, NULL, ev->day, 0, APOINT_NULL);
	io_set_modified();
}
