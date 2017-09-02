#!/usr/bin/env python3

import argparse
import datetime


def get_date(s):
    return datetime.datetime.strptime(s, '%Y-%m-%d').date()


parser = argparse.ArgumentParser('calcurse-dateutil')
parser.add_argument('--date', type=get_date, action='store', dest='date')
parser.add_argument('--range', type=int, action='store', dest='range')
parser.add_argument('--unique', action='store_true', dest='unique')
parser.add_argument('--append', type=str, action='store', dest='append')
parser.add_argument('op', nargs='+')
args = parser.parse_args()


def skip_days(d, n):
    return d + datetime.timedelta(days=n)


def skip_months(d, n):
    return d + datetime.timedelta(months=1)


def next_weekday(d, w):
    return skip_days(d, (w - d.weekday() + 7) % 7)


def bow(d):
    return skip_days(d, -d.weekday())


def bom(d):
    return d.replace(day=1)


def eow(d):
    return skip_days(bow(d), 6)


def eom(d):
    return skip_months(bom(d), 1)


s = args.date if args.date else datetime.date.today()
r = args.range if args.range else 1
a = args.append if args.append else ''
seen = set()

for i in range(0, r):
    d = skip_days(s, i)
    it = iter(args.op)
    for arg in it:
        if arg == 'bow':
            d = bow(d)
        elif arg == 'bom':
            d = bom(d)
        elif arg == 'eow':
            d = eow(d)
        elif arg == 'eom':
            d = eom(d)
        elif arg == 'next-weekday':
            d = next_weekday(d, int(next(it)))
        elif arg == 'skip-days':
            d = skip_days(d, int(next(it)))
        elif arg == 'skip-months':
            d = skip_months(d, int(next(it)))
    out = "{}{}".format(d, a)
    if args.unique:
        if d not in seen:
            print(out)
            seen.add(d)
    else:
        print(out)
