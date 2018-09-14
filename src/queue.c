#include "calcurse.h"

/*
 * A queue for calcurse system messages.
 */
llist_t sysqueue;

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
	LLIST_ADD(&sysqueue, ev);

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
	else
		ev = sysqueue.head->data;
	mem_free(ev->mesg);
	mem_free(ev);
	LLIST_REMOVE(&sysqueue, sysqueue.head);
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
