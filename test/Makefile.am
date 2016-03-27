AUTOMAKE_OPTIONS = foreign

TESTS = \
	true-001.sh \
	run-test-001.sh \
	run-test-002.sh \
	todo-001.sh \
	todo-002.sh \
	todo-003.sh \
	day-001.sh \
	day-002.sh \
	day-003.sh \
	range-001.sh \
	range-002.sh \
	range-003.sh \
	appointment-001.sh \
	appointment-002.sh \
	appointment-003.sh \
	appointment-004.sh \
	appointment-005.sh \
	appointment-006.sh \
	appointment-007.sh \
	appointment-008.sh \
	appointment-009.sh \
	appointment-010.sh \
	appointment-011.sh \
	appointment-012.sh \
	appointment-013.sh \
	appointment-014.sh \
	appointment-015.sh \
	appointment-016.sh \
	appointment-017.sh \
	appointment-018.sh \
	appointment-019.sh \
	appointment-020.sh \
	appointment-021.sh \
	appointment-022.sh \
	event-001.sh \
	event-002.sh \
	event-003.sh \
	event-004.sh \
	event-005.sh \
	event-006.sh \
	filter-001.sh \
	ical-001.sh \
	ical-002.sh \
	ical-003.sh \
	ical-004.sh \
	ical-005.sh \
	ical-006.sh \
	ical-007.sh \
	next-001.sh \
	search-001.sh \
	bug-002.sh \
	regress-001.sh \
	recur-001.sh \
	recur-002.sh \
	recur-003.sh \
	recur-004.sh \
	recur-005.sh

TESTS_ENVIRONMENT = \
	TEST_INIT='$(top_srcdir)/test/test-init.sh' \
	CALCURSE='$(top_builddir)/src/calcurse' \
	DATA_DIR='$(top_srcdir)/test/data/'

AM_CFLAGS = -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L

check_PROGRAMS = run-test
check_SCRIPTS = test-init.sh
noinst_SCRIPTS = $(check_SCRIPTS)

run_test_SOURCES = run-test.c

EXTRA_DIST = \
	$(TESTS) \
	test-init.sh \
	data/apts \
	data/apts-appointment-002 \
	data/apts-appointment-003 \
	data/apts-appointment-004 \
	data/apts-appointment-005 \
	data/apts-appointment-006 \
	data/apts-appointment-007 \
	data/apts-appointment-008 \
	data/apts-appointment-009 \
	data/apts-appointment-010 \
	data/apts-appointment-011 \
	data/apts-appointment-012 \
	data/apts-appointment-013 \
	data/apts-appointment-014 \
	data/apts-appointment-015 \
	data/apts-appointment-016 \
	data/apts-appointment-017 \
	data/apts-appointment-018 \
	data/apts-appointment-019 \
	data/apts-appointment-020 \
	data/apts-appointment-021 \
	data/apts-appointment-022 \
	data/apts-bug-002 \
	data/apts-event-001 \
	data/apts-event-002 \
	data/apts-event-003 \
	data/apts-event-004 \
	data/apts-event-005 \
	data/apts-event-006 \
	data/apts-filter-001 \
	data/apts-recur \
	data/apts-regress-001 \
	data/conf \
	data/ical-001.ical \
	data/ical-002.ical \
	data/ical-003.ical \
	data/ical-004.ical \
	data/ical-005.ical \
	data/ical-006.ical \
	data/ical-007.ical \
	data/todo
