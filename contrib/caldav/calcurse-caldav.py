#!/usr/bin/python3

import argparse
import base64
import configparser
import http.client
import os
import re
import ssl
import subprocess
import sys
import textwrap
import xml.etree.ElementTree as etree


def msgfmt(msg, prefix=''):
    lines = []
    for line in msg.splitlines():
        lines += textwrap.wrap(line, 80 - len(prefix))
    return '\n'.join([prefix + line for line in lines])


def warn(msg):
    print(msgfmt(msg, "warning: "))


def die(msg):
    sys.exit(msgfmt(msg, "error: "))


def die_atnode(msg, node):
    if debug:
        msg += '\n\n'
        msg += 'The error occurred while processing the following XML node:\n'
        msg += etree.tostring(node).decode('utf-8')
    die(msg)


def calcurse_wipe():
    if verbose:
        print('Removing all local calcurse objects...')
    if dry_run:
        return
    subprocess.call([calcurse, '-F', '--filter-hash=XXX'])


def calcurse_import(icaldata):
    p = subprocess.Popen([calcurse, '-i', '-', '--dump-imported', '-q',
                          '--format-recur-apt=%(hash)\\n',
                          '--format-event=%(hash)\\n',
                          '--format-recur-event=%(hash)\\n',
                          '--format-todo=%(hash)\\n'],
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    return p.communicate(icaldata.encode('utf-8'))[0].decode('utf-8').rstrip()


def calcurse_export(objhash):
    p = subprocess.Popen([calcurse, '-x', 'ical', '--export-uid',
                          '--filter-hash=' + objhash], stdout=subprocess.PIPE)
    return p.communicate()[0].decode('utf-8').rstrip()


def calcurse_hashset():
    p = subprocess.Popen([calcurse, '-G', '--format-apt=%(hash)\\n',
                          '--format-recur-apt=%(hash)\\n',
                          '--format-event=%(hash)\\n',
                          '--format-recur-event=%(hash)\\n',
                          '--format-todo=%(hash)\\n'], stdout=subprocess.PIPE)
    return set(p.communicate()[0].decode('utf-8').rstrip().splitlines())


def calcurse_remove(objhash):
    subprocess.call([calcurse, '-F', '--filter-hash=!' + objhash])


def calcurse_version():
    p = subprocess.Popen([calcurse, '--version'], stdout=subprocess.PIPE)
    m = re.match(r'calcurse ([0-9]+)\.([0-9]+)\.([0-9]+)(?:-([0-9]+)-)?',
                 p.communicate()[0].decode('utf-8'))
    if not m:
        return None
    return tuple([int(group) for group in m.groups(0)])


def get_auth_headers():
    if not username or not password:
        return {}
    user_password = ('{}:{}'.format(username, password)).encode('ascii')
    user_password = base64.b64encode(user_password).decode('ascii')
    headers = {'Authorization': 'Basic {}'.format(user_password)}
    return headers


def remote_query(conn, cmd, path, additional_headers, body):
    headers = custom_headers.copy()
    headers.update(get_auth_headers())
    if cmd == 'PUT':
        headers['Content-Type'] = 'text/calendar; charset=utf-8'
    else:
        headers['Content-Type'] = 'application/xml; charset=utf-8'
    headers.update(additional_headers)

    if debug:
        print("> {} {}".format(cmd, path))
        print("> Headers: " + repr(headers))
        if body:
            for line in body.splitlines():
                print("> " + line)
        print()

    if isinstance(body, str):
        body = body.encode('utf-8')

    conn.request(cmd, path, headers=headers, body=body)

    resp = conn.getresponse()

    if not resp:
        return (None, None)

    headers = resp.getheaders()
    body = resp.read().decode('utf-8')

    if debug:
        print("< Headers: " + repr(headers))
        for line in body.splitlines():
            print("< " + line)
        print()

    if resp.status - (resp.status % 100) != 200:
        die(("The server at {} replied with HTTP status code {} ({}) " +
             "while trying to access {}.").format(hostname, resp.status,
                                                  resp.reason, path))

    return (headers, body)


def get_etags(conn, hrefs=[]):
    if len(hrefs) > 0:
        body = ('<?xml version="1.0" encoding="utf-8" ?>'
                '<C:calendar-multiget xmlns:D="DAV:" '
                '                     xmlns:C="urn:ietf:params:xml:ns:caldav">'
                '<D:prop><D:getetag /></D:prop>')
        for href in hrefs:
            body += '<D:href>{}</D:href>'.format(href)
        body += '</C:calendar-multiget>'
    else:
        body = ('<?xml version="1.0" encoding="utf-8" ?>'
                '<C:calendar-query xmlns:D="DAV:" '
                '                  xmlns:C="urn:ietf:params:xml:ns:caldav">'
                '<D:prop><D:getetag /></D:prop>'
                '<C:filter><C:comp-filter name="VCALENDAR" /></C:filter>'
                '</C:calendar-query>')
    headers, body = remote_query(conn, "REPORT", path, {}, body)
    if not headers:
        return {}
    root = etree.fromstring(body)

    etagdict = {}
    for node in root.findall(".//D:response", namespaces=nsmap):
        etagnode = node.find("./D:propstat/D:prop/D:getetag", namespaces=nsmap)
        if etagnode is None:
            die_atnode('Missing ETag.', node)
        etag = etagnode.text.strip('"')

        hrefnode = node.find("./D:href", namespaces=nsmap)
        if hrefnode is None:
            die_atnode('Missing href.', node)
        href = hrefnode.text

        etagdict[href] = etag

    return etagdict


def remote_wipe(conn):
    if verbose:
        print('Removing all objects from the CalDAV server...')
    if dry_run:
        return

    remote_query(conn, "DELETE", path, {}, None)


def get_syncdb(fn):
    if not os.path.exists(fn):
        return {}

    if verbose:
        print('Loading synchronization database from ' + fn + '...')

    syncdb = {}
    with open(fn, 'r') as f:
        for line in f.readlines():
            href, etag, objhash = line.rstrip().split(' ')
            syncdb[href] = (etag, objhash)

    return syncdb


def save_syncdb(fn, syncdb):
    if verbose:
        print('Saving synchronization database to ' + fn + '...')
    if dry_run:
        return

    with open(fn, 'w') as f:
        for href, (etag, objhash) in syncdb.items():
            print("{} {} {}".format(href, etag, objhash), file=f)


def push_object(conn, objhash):
    href = path + objhash + ".ics"
    body = calcurse_export(objhash)
    headers, body = remote_query(conn, "PUT", href, {}, body)

    if not headers:
        return None

    etag = None
    headerdict = dict((key.lower(), value) for key, value in headers)
    if 'etag' in headerdict:
        etag = headerdict['etag']
    while not etag:
        etagdict = get_etags(conn, [href])
        if etagdict:
            etag = next(iter(etagdict.values()))
    etag = etag.strip('"')

    return (href, etag)


def remove_remote_object(conn, etag, href):
    headers = {'If-Match': '"' + etag + '"'}
    remote_query(conn, "DELETE", href, headers, None)


def push_objects(conn, syncdb, etagdict):
    objhashes = calcurse_hashset()
    new = objhashes - set([entry[1] for entry in syncdb.values()])
    gone = set([entry[1] for entry in syncdb.values()]) - objhashes

    added = deleted = 0

    # Copy new objects to the server.
    for objhash in new:
        if verbose:
            print("Pushing new object {} to the server.".format(objhash))
        if dry_run:
            continue

        href, etag = push_object(conn, objhash)
        syncdb[href] = (etag, objhash)
        added += 1

    # Remove locally deleted objects from the server.
    for objhash in gone:
        queue = []
        for href, entry in syncdb.items():
            if entry[1] == objhash:
                queue.append(href)

        for href in queue:
            etag = syncdb[href][0]

            if etagdict[href] != etag:
                warn(('{} was deleted locally but modified in the CalDAV '
                      'calendar. Keeping the modified version on the server. '
                      'Run the script again to import the modified '
                      'object.').format(objhash))
                syncdb.pop(href, None)
                continue

            if verbose:
                print("Removing remote object {} ({}).".format(etag, href))
            if dry_run:
                continue

            remove_remote_object(conn, etag, href)
            syncdb.pop(href, None)
            deleted += 1

    return (added, deleted)


def pull_objects(conn, syncdb, etagdict):
    missing = set()
    modified = set()
    for href in set(etagdict.keys()):
        if href not in syncdb:
            missing.add(href)
        elif etagdict[href] != syncdb[href][0]:
            modified.add(href)
    orphan = set(syncdb.keys()) - set(etagdict.keys())

    # Download and import new objects from the server.
    body = ('<?xml version="1.0" encoding="utf-8" ?>'
            '<C:calendar-multiget xmlns:D="DAV:" '
            '                     xmlns:C="urn:ietf:params:xml:ns:caldav">'
            '<D:prop><D:getetag /><C:calendar-data /></D:prop>')
    for href in (missing | modified):
        body += '<D:href>{}</D:href>'.format(href)
    body += '</C:calendar-multiget>'
    headers, body = remote_query(conn, "REPORT", path, {}, body)

    root = etree.fromstring(body)

    added = deleted = 0

    for node in root.findall(".//D:response", namespaces=nsmap):
        hrefnode = node.find("./D:href", namespaces=nsmap)
        if hrefnode is None:
            die_atnode('Missing href.', node)
        href = hrefnode.text

        etagnode = node.find("./D:propstat/D:prop/D:getetag", namespaces=nsmap)
        if etagnode is None:
            die_atnode('Missing ETag.', node)
        etag = etagnode.text.strip('"')

        cdatanode = node.find("./D:propstat/D:prop/C:calendar-data",
                              namespaces=nsmap)
        if cdatanode is None:
            die_atnode('Missing calendar data.', node)
        cdata = cdatanode.text

        if href in modified:
            if verbose:
                print("Replacing object {}.".format(etag))
            if dry_run:
                continue
            objhash = syncdb[href][1]
            calcurse_remove(objhash)
        else:
            if verbose:
                print("Importing new object {}.".format(etag))
            if dry_run:
                continue

        objhash = calcurse_import(cdata)
        syncdb[href] = (etag, objhash)
        added += 1

    # Delete objects that no longer exist on the server.
    for href in orphan:
        etag, objhash = syncdb[href]

        if verbose:
            print("Removing local object {}.".format(objhash))
        if dry_run:
            continue

        calcurse_remove(objhash)
        syncdb.pop(href, None)
        deleted += 1

    return (added, deleted)


def run_hook(name):
    hook_path = hookdir + '/' + name
    if not os.path.exists(hook_path):
        return
    subprocess.call(hook_path, shell=True)


# Initialize the XML namespace map.
nsmap = {"D": "DAV:", "C": "urn:ietf:params:xml:ns:caldav"}

# Initialize default values.
configfn = os.path.expanduser("~/.calcurse/caldav/config")
lockfn = os.path.expanduser("~/.calcurse/caldav/lock")
syncdbfn = os.path.expanduser("~/.calcurse/caldav/sync.db")
hookdir = os.path.expanduser("~/.calcurse/caldav/hooks/")

# Parse command line arguments.
parser = argparse.ArgumentParser('calcurse-caldav')
parser.add_argument('--init', action='store', dest='init', default=None,
                    choices=['keep-remote', 'keep-local', 'two-way'],
                    help='initialize the sync database')
parser.add_argument('--config', action='store', dest='configfn',
                    default=configfn,
                    help='path to the calcurse-caldav configuration')
parser.add_argument('--lockfile', action='store', dest='lockfn',
                    default=lockfn,
                    help='path to the calcurse-caldav lock file')
parser.add_argument('--syncdb', action='store', dest='syncdbfn',
                    default=syncdbfn,
                    help='path to the calcurse-caldav sync DB')
parser.add_argument('--hookdir', action='store', dest='hookdir',
                    default=hookdir,
                    help='path to the calcurse-caldav hooks directory')
parser.add_argument('-v', '--verbose', action='store_true', dest='verbose',
                    default=False,
                    help='print status messages to stdout')
parser.add_argument('--debug', action='store_true', dest='debug',
                    default=False, help='print debug messages to stdout')
args = parser.parse_args()

init = args.init is not None
configfn = args.configfn
lockfn = args.lockfn
syncdbfn = args.syncdbfn
hookdir = args.hookdir
verbose = args.verbose
debug = args.debug

# Read configuration.
config = configparser.RawConfigParser()
if verbose:
    print('Loading configuration from ' + configfn + '...')
try:
    config.readfp(open(configfn))
except FileNotFoundError as e:
    die('Configuration file not found: {}'.format(configfn))

hostname = config.get('General', 'HostName')
path = '/' + config.get('General', 'Path').strip('/') + '/'

if config.has_option('General', 'InsecureSSL'):
    insecure_ssl = config.getboolean('General', 'InsecureSSL')
else:
    insecure_ssl = False

if config.has_option('General', 'Binary'):
    calcurse = config.get('General', 'Binary')
else:
    calcurse = 'calcurse'

if config.has_option('General', 'DryRun'):
    dry_run = config.getboolean('General', 'DryRun')
else:
    dry_run = True

if not verbose and config.has_option('General', 'Verbose'):
    verbose = config.getboolean('General', 'Verbose')

if not debug and config.has_option('General', 'Debug'):
    debug = config.getboolean('General', 'Debug')

if config.has_option('Auth', 'UserName'):
    username = config.get('Auth', 'UserName')
else:
    username = None

if config.has_option('Auth', 'Password'):
    password = config.get('Auth', 'Password')
else:
    password = None

if config.has_section('CustomHeaders'):
    custom_headers = dict(config.items('CustomHeaders'))
else:
    custom_headers = {}

# Show disclaimer when performing a dry run.
if dry_run:
    warn(('Dry run; nothing is imported/exported. Add "DryRun = No" to the '
          '[General] section in the configuration file to enable '
          'synchronization.'))

# Check whether the specified calcurse binary is executable and compatible.
ver = calcurse_version()
if ver is None:
    die('Invalid calcurse binary. Make sure that the file specified in ' +
        'the configuration is a valid and up-to-date calcurse binary.')
elif ver < (4, 0, 0, 96):
    die('Incompatible calcurse binary detected. Version >=4.1.0 is required ' +
        'to synchronize with CalDAV servers.')

# Run the pre-sync hook.
run_hook('pre-sync')

# Create lock file.
if os.path.exists(lockfn):
    die('Leftover lock file detected. If there is no other synchronization ' +
        'instance running, please remove the lock file manually and try ' +
        'again.')
open(lockfn, 'w')

try:
    # Connect to the server via HTTPs.
    if verbose:
        print('Connecting to ' + hostname + '...')
    if insecure_ssl:
        try:
            context = ssl._create_unverified_context()
            conn = http.client.HTTPSConnection(hostname, context=context)
        except AttributeError:
            # Python versions prior to 3.4.3 do not support
            # ssl._create_unverified_context(). However, these versions do not
            # seem to verify certificates by default so we can simply fall back
            # to http.client.HTTPSConnection().
            conn = http.client.HTTPSConnection(hostname)
    else:
        conn = http.client.HTTPSConnection(hostname)

    if init:
        # In initialization mode, start with an empty synchronization database.
        if args.init == 'keep-remote':
            calcurse_wipe()
        elif args.init == 'keep-local':
            remote_wipe(conn)
        syncdb = {}
    else:
        # Read the synchronization database.
        syncdb = get_syncdb(syncdbfn)

        if not syncdb:
            die('Sync database not found or empty. Please initialize the ' +
                'database first.\n\nSupported initialization modes are:\n' +
                '  --init=keep-remote Remove all local calcurse items\n' +
                '  --init=keep-local  Remove all remote objects\n' +
                '  --init=two-way     Copy local items to the server and vice versa')

    # Query the server and build a dictionary that maps ETags to paths on the
    # server.
    etagdict = get_etags(conn)

    # Retrieve new objects from the server, delete local items that no longer
    # exist on the server.
    local_new, local_del = pull_objects(conn, syncdb, etagdict)

    # Push new objects to the server, remove items from the server if they no
    # longer exist locally.
    remote_new, remote_del = push_objects(conn, syncdb, etagdict)

    # Write the synchronization database.
    save_syncdb(syncdbfn, syncdb)

    # Close the HTTPs connection.
    conn.close()
finally:
    # Remove lock file.
    os.remove(lockfn)

# Run the post-sync hook.
run_hook('post-sync')

# Print a summary to stdout.
print("{} items imported, {} items removed locally.".
      format(local_new, local_del))
print("{} items exported, {} items removed from the server.".
      format(remote_new, remote_del))
