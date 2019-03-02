#!/usr/bin/env python3

import argparse
import io
import os
import re
import subprocess
import sys
import textwrap


def msgfmt(msg, prefix=''):
    """Print a formatted message"""
    lines = []
    for line in msg.splitlines():
        lines += textwrap.wrap(line, 80 - len(prefix))
    return '\n'.join([prefix + line for line in lines])


def check_binary(binary):
    """Check if a binary is available in $PATH"""
    try:
        subprocess.call([binary, '--version'], stdout=subprocess.DEVNULL)
    except FileNotFoundError:
        sys.exit(msgfmt("{0} is not available.".format(binary), "error: "))


def check_directory(directory):
    """Check if a directory exists"""
    if not os.path.isdir(directory):
        sys.exit(msgfmt("invalid directory: {0}".format(directory), "error: "))


def write_file(file, contents):
    """Write to file"""
    if verbose:
        msgfmt("Writing to file {0}".format(file))
    with open(file, 'w') as f:
        f.write(contents)


def remove_file(file):
    """Remove file"""
    if verbose:
        msgfmt("Deleting file {0}".format(file))
    if os.path.isfile(file):
        os.remove(file)


def calcurse_export():
    """Return raw calcurse data"""
    command = calcurse + ['-xical', '--export-uid']
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    return [x for x in io.TextIOWrapper(proc.stdout, encoding="utf-8")]


def calcurse_remove(uid):
    """Remove calcurse event by uid"""
    command = calcurse + ['-P', '--filter-hash=' + uid]
    subprocess.call(command)


def calcurse_import(file):
    """Import ics file to calcurse"""
    command = calcurse + ['-i', file]
    subprocess.call(command)


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


def vdirsyncer_sync():
    """Sync vdir using vdirsyncer"""
    command = vdirsyncer + ['sync']
    if force:
        command += ['--force-delete']
    subprocess.call(command)


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

    files_old = [x for x in os.listdir(vdir)]
    files_new = [uid + ".ics" for uid in events]

    if not files_new == 0 and not force:
        sys.exit(msgfmt("""no calcurse events were exported."""
                        """ Use --force to synchronize anyways.""", 'error: '))

    for file in files_old:
        if file not in files_new:
            remove_file(os.path.join(vdir, file))

    for uid, event in events.items():
        file = uid + ".ics"
        if file not in files_old:
            write_file(os.path.join(vdir, file), event)


def vdir_to_calcurse():
    """Import vdir data to calcurse"""
    files_old = [x + '.ics' for x in calcurse_list()]
    files_new = [x for x in os.listdir(vdir) if x.endswith('.ics')]

    if not files_new == 0 and not force:
        sys.exit(msgfmt("""the vdirsyncer directory is empty."""
                        """ Use --force to synchronize anyways.""", 'error: '))

    for file in files_new:
        if file not in files_old:
            calcurse_import(os.path.join(vdir, file))

    for file in files_old:
        if file not in files_new:
            calcurse_remove(file[:-4])


parser = argparse.ArgumentParser('calcurse-vdirsyncer')
parser.add_argument('vdir',
                    help='path to the vdir collection directory')
parser.add_argument('-D', action='store', dest='datadir',
                    default=None,
                    help='path to the calcurse data directory')
parser.add_argument('-v', '--verbose', action='store_true', dest='verbose',
                    default=False,
                    help='print status messages to stdout')
parser.add_argument('-f', '--force', action='store_true', dest='force',
                    default=False,
                    help='for synchronization of empty collections')
args = parser.parse_args()

datadir = args.datadir
force = args.force
verbose = args.verbose
vdir = args.vdir


check_binary('calcurse')
check_binary('vdirsyncer')

calcurse = ['calcurse']
vdirsyncer = ['vdirsyncer']

check_directory(vdir)

if datadir:
    check_directory(datadir)
    calcurse += ['-D', datadir]

calcurse_to_vdir()
vdirsyncer_sync()
vdir_to_calcurse()
