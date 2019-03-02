#!/usr/bin/env python3

import argparse
import io
import os
import re
import subprocess
import sys
import textwrap


def msgfmt(msg, prefix=''):
    """Format a message"""
    lines = []
    for line in msg.splitlines():
        lines += textwrap.wrap(line, 80 - len(prefix))
    return '\n'.join([prefix + line for line in lines])


def log(msg):
    """Print a formatted message"""
    print(msgfmt(msg))


def die(msg):
    """Exit on error"""
    sys.exit(msgfmt(msg, prefix="error: "))


def check_binary(binary):
    """Check if a binary is available in $PATH"""
    try:
        subprocess.call([binary, '--version'], stdout=subprocess.DEVNULL)
    except FileNotFoundError:
        die("{0} is not available.".format(binary))


def check_directory(directory):
    """Check if a directory exists"""
    if not os.path.isdir(directory):
        die("invalid directory: {0}".format(directory))


def file_to_uid(file):
    """Return the uid of an ical file"""
    uid = file.replace(vdir, "").replace(".ics", "")
    return uid


def write_file(file, contents):
    """Write to file"""
    if verbose:
        log("Writing event {0}".format(file_to_uid(file)))
    with open(file, 'w') as f:
        f.write(contents)


def remove_file(file):
    """Remove file"""
    if verbose:
        log("Deleting event {0}".format(file_to_uid(file)))
    if os.path.isfile(file):
        os.remove(file)


def calcurse_export():
    """Return raw calcurse data"""
    command = calcurse + ['-xical', '--export-uid']
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    return [x for x in io.TextIOWrapper(proc.stdout, encoding="utf-8")]


def calcurse_remove(uid):
    """Remove calcurse event by uid"""
    if verbose:
        log("Removing event {0} from calcurse".format(uid))
    command = calcurse + ['-P', '--filter-hash=' + uid]
    subprocess.call(command)


def calcurse_import(file):
    """Import ics file to calcurse"""
    if verbose:
        log("Importing event {0} to calcurse".format(file_to_uid(file)))
    command = calcurse + ['-i', file]
    subprocess.call(command, stdout=subprocess.DEVNULL)


def calcurse_list():
    """Return all calcurse item uids"""
    command = calcurse + [
        '-G',
        '--format-apt=%(hash)\\n',
        '--format-recur-apt=%(hash)\\n',
        '--format-event=%(hash)\\n',
        '--format-recur-event=%(hash)\\n',
        '--format-todo=%(hash)\\n'
    ]
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    return [x.strip() for x in io.TextIOWrapper(proc.stdout, encoding="utf-8")]


def parse_calcurse_data(raw):
    """Parse raw calcurse data to a uid/ical dictionary"""

    header = ''.join(raw[:3])
    regex = '(BEGIN:(VEVENT|VTODO).*?END:(VEVENT|VTODO).)'
    events = [x[0] for x in re.findall(regex, ''.join(raw), re.DOTALL)]

    items = {}

    for item in events:
        uid = re.findall('UID:(.*?)\n', item)[0]
        items[uid] = header + item + "END:VCALENDAR\n"

    return items


def calcurse_to_vdir():
    """Export calcurse data to vdir"""
    raw_events = calcurse_export()
    events = parse_calcurse_data(raw_events)

    files_vdir = [x for x in os.listdir(vdir)]
    files_calc = [uid + ".ics" for uid in events]

    if force:
        for file in [f for f in files_vdir if f not in files_calc]:
            remove_file(os.path.join(vdir, file))

    for uid, event in events.items():
        file = uid + ".ics"
        if file not in files_vdir:
            write_file(os.path.join(vdir, file), event)


def vdir_to_calcurse():
    """Import vdir data to calcurse"""
    files_calc = [x + '.ics' for x in calcurse_list()]
    files_vdir = [x for x in os.listdir(vdir) if x.endswith('.ics')]

    for file in [f for f in files_vdir if f not in files_calc]:
        calcurse_import(os.path.join(vdir, file))

    if force:
        for file in [f for f in files_calc if f not in files_vdir]:
            calcurse_remove(file[:-4])


parser = argparse.ArgumentParser('calcurse-vdir')
parser.add_argument('action', choices=['import', 'export'],
                    help='export or import calcurse data')
parser.add_argument('vdir',
                    help='path to the vdir collection directory')
parser.add_argument('-D', '--datadir', action='store', dest='datadir',
                    default=None,
                    help='path to the calcurse data directory')
parser.add_argument('-f', '--force', action='store_true', dest='force',
                    default=False,
                    help='enable destructive import and export')
parser.add_argument('-v', '--verbose', action='store_true', dest='verbose',
                    default=False,
                    help='print status messages to stdout')
args = parser.parse_args()

action = args.action
datadir = args.datadir
force = args.force
verbose = args.verbose
vdir = args.vdir

check_directory(vdir)

check_binary('calcurse')
calcurse = ['calcurse']

if datadir:
    check_directory(datadir)
    calcurse += ['-D', datadir]

if action == 'import':
    vdir_to_calcurse()
elif action == 'export':
    calcurse_to_vdir()
else:
    die("Invalid action {0}.".format(action))
