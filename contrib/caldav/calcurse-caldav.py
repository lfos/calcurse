#!/usr/bin/python3

import argparse
import base64
import configparser
import hashlib
import http.client
import os
import ssl
import subprocess
import sys
import textwrap
import xml.etree.ElementTree as etree

def die(msg):
    newmsg = ""
    for line in msg.splitlines():
        newmsg += textwrap.fill(line, 72) + '\n'
    msg = ""
    for line in newmsg.splitlines():
        msg += 'error: ' + line + '\n'
    sys.exit(msg.rstrip('\n'))

def die_atnode(msg, node):
    if verbose:
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
    p = subprocess.Popen([calcurse, '-i', '-', '--list-imported', '-q'],
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    return p.communicate(icaldata.encode('utf-8'))[0].decode('utf-8').rstrip()

def calcurse_export(objhash):
    p = subprocess.Popen([calcurse, '-x', 'ical', '--export-uid',
                          '--filter-hash=' + objhash], stdout=subprocess.PIPE)
    return p.communicate()[0].decode('utf-8').rstrip()

def calcurse_hashset():
    p = subprocess.Popen([calcurse, '-G'], stdout=subprocess.PIPE)
    out = p.communicate()[0]

    hashes = set()
    for line in out.split(b'\n'):
        if not line:
            continue
        sha1 = hashlib.new('sha1')
        sha1.update(line)
        hashes.add(sha1.hexdigest())
    return hashes

def calcurse_remove(objhash):
    subprocess.call([calcurse, '-F', '--filter-hash=!' + objhash])

def calcurse_version():
    p = subprocess.Popen([calcurse, '--version'], stdout=subprocess.PIPE)
    tokens = p.communicate()[0].decode('utf-8').rstrip().split(" ")

    if len(tokens) < 2:
        return None
    if tokens[0] != 'Calcurse':
        return None
    tokens = tokens[1].split(".")
    if len(tokens) < 2:
        return None
    return int(tokens[0]) * 10 + int(tokens[1])

def get_auth_headers():
    if not username or not password:
        return {}
    user_password = ('%s:%s' % (username, password)).encode('ascii')
    user_password = base64.b64encode(user_password).decode('ascii')
    headers = { 'Authorization' : 'Basic %s' % user_password }
    return headers

def remote_query(cmd, path, additional_headers, body):
    headers = custom_headers.copy()
    headers.update(get_auth_headers())
    if cmd == 'PUT':
        headers['Content-Type'] = 'text/calendar; charset=utf-8'
    else:
        headers['Content-Type'] = 'application/xml; charset=utf-8'
    headers.update(additional_headers)

    if debug:
        print("> %s %s" % (cmd, path))
        print("> Headers: " + repr(headers))
        if body:
            for line in body.splitlines():
                print("> " + line)
        print()

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
        die(("The server at %s replied with HTTP status code %d (%s) " +
             "while trying to access %s.") %
            (hostname, resp.status, resp.reason, path))

    return (headers, body)

def get_hrefmap(conn, uid=None):
    if uid:
        propfilter = '<C:prop-filter name="UID">' +\
                     '<C:text-match collation="i;octet" >%s</C:text-match>' +\
                     '</C:prop-filter>' % (uid)
    else:
        propfilter = ''

    body = '<?xml version="1.0" encoding="utf-8" ?>' +\
           '<C:calendar-query xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav">' +\
           '<D:prop><D:getetag /></D:prop><C:filter>' +\
           '<C:comp-filter name="VCALENDAR">' + propfilter + '</C:comp-filter>' +\
           '</C:filter></C:calendar-query>'
    headers, body = remote_query("REPORT", path, {}, body)
    if not headers:
        return {}
    root = etree.fromstring(body)

    hrefmap = {}
    for node in root.findall(".//D:response", namespaces=nsmap):
        etagnode = node.find("./D:propstat/D:prop/D:getetag", namespaces=nsmap)
        if etagnode is None:
            die_atnode('Missing ETag.', node)
        etag = etagnode.text.strip('"')

        hrefnode = node.find("./D:href", namespaces=nsmap)
        if hrefnode is None:
            die_atnode('Missing href.', node)
        href = hrefnode.text

        hrefmap[etag] = href

    return hrefmap

def remote_wipe(conn):
    if verbose:
        print('Removing all objects from the CalDAV server...')
    if dry_run:
        return

    remote_query("DELETE", path, {}, None)

def get_syncdb(fn):
    if not os.path.exists(fn):
        return {}

    if verbose:
        print('Loading synchronization database from ' + fn + '...')

    syncdb = {}
    with open(fn, 'r') as f:
        for line in f.readlines():
            etag, objhash = line.rstrip().split(' ')
            syncdb[etag] = objhash

    return syncdb

def save_syncdb(fn, syncdb):
    if verbose:
        print('Saving synchronization database to ' + fn + '...')
    if dry_run:
        return

    with open(fn, 'w') as f:
        for etag, objhash in syncdb.items():
            print("%s %s" % (etag, objhash), file=f)

def push_object(conn, objhash):
    body = calcurse_export(objhash)
    headers, body = remote_query("PUT", path + objhash + ".ics", {}, body)

    if not headers:
        return None

    etag = [header[1] for header in headers if header[0] == 'ETag'][0]
    while not etag:
        hrefmap = get_hrefmap(conn, objhash)
        if len(hrefmap.keys()) > 0:
            etag = hrefmap.keys()[0]
    etag = etag.strip('"')

    return etag

def remove_remote_object(conn, etag, href):
    headers = { 'If-Match' : '"' + etag + '"' }
    remote_query("DELETE", href, headers, None)

def push_objects(conn, syncdb, hrefmap):
    objhashes = calcurse_hashset()
    new = objhashes - set(syncdb.values())
    gone = set(syncdb.values()) - objhashes

    added = deleted = 0

    # Copy new objects to the server.
    for objhash in new:
        if verbose:
            print("Pushing new object %s to the server." % (objhash))
        if dry_run:
            continue

        etag = push_object(conn, objhash)
        syncdb[etag] = objhash
        added += 1

    # Remove locally deleted objects from the server.
    for objhash in gone:
        deletags = []
        for key, value in syncdb.items():
            if value == objhash:
                deletags.append(key)

        for etag in deletags:
            if not etag in hrefmap:
                continue
            href = hrefmap[etag]

            if verbose:
                print("Removing remote object %s (%s)." % (etag, href))
            if dry_run:
                continue

            remove_remote_object(conn, etag, href)
            syncdb.pop(etag, None)
            deleted += 1

    return (added, deleted)

def pull_objects(conn, syncdb, hrefmap):
    missing = set(hrefmap.keys()) - set(syncdb.keys())
    orphan = set(syncdb.keys()) - set(hrefmap.keys())

    # Download and import new objects from the server.
    body = '<?xml version="1.0" encoding="utf-8" ?>' +\
           '<C:calendar-multiget xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:caldav">' +\
           '<D:prop><D:getetag /><C:calendar-data /></D:prop>'
    for etag in missing:
        body += '<D:href>%s</D:href>' % (hrefmap[etag])
    body += '</C:calendar-multiget>'
    headers, body = remote_query("REPORT", path, {}, body)

    root = etree.fromstring(body)

    added = deleted = 0

    for node in root.findall(".//D:prop", namespaces=nsmap):
        etagnode = node.find("./D:getetag", namespaces=nsmap)
        if etagnode is None:
            die_atnode('Missing ETag.', node)
        etag = etagnode.text.strip('"')

        cdatanode = node.find("./C:calendar-data", namespaces=nsmap)
        if cdatanode is None:
            die_atnode('Missing calendar data.', node)
        cdata = cdatanode.text

        if verbose:
            print("Importing new object %s." % (etag))
        if dry_run:
            continue

        objhash = calcurse_import(cdata)
        syncdb[etag] = objhash
        added += 1

    # Delete objects that no longer exist on the server.
    for etag in orphan:
        objhash = syncdb[etag]

        if verbose:
            print("Removing local object %s." % (objhash))
        if dry_run:
            continue

        calcurse_remove(objhash)
        syncdb.pop(etag, None)
        deleted += 1

    return (added, deleted)

# Initialize the XML namespace map.
nsmap = { "D": "DAV:", "C": "urn:ietf:params:xml:ns:caldav" }

# Initialize default values.
configfn = os.path.expanduser("~/.calcurse/caldav/config")
lockfn = os.path.expanduser("~/.calcurse/caldav/lock")
syncdbfn = os.path.expanduser("~/.calcurse/caldav/sync.db")

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
verbose = args.verbose
debug = args.debug

# Read configuration.
config = configparser.RawConfigParser()
if verbose:
    print('Loading configuration from ' + configfn + '...')
try:
    config.readfp(open(configfn))
except FileNotFoundError as e:
    die('Configuration file not found: %s' % (configfn))

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
    print('Dry run. Nothing is actually imported/exported.')
    print('Edit the configuration file and add "DryRun = No" to the [General] section')
    print('to enable synchronization.')

# Check whether the specified calcurse binary is executable and compatible.
ver = calcurse_version()
if ver is None:
    die('Invalid calcurse binary. Make sure that the file specified in ' +
        'the configuration is a valid and up-to-date calcurse binary.')
elif ver < 40:
    die('Incompatible calcurse binary detected. Version >=4.0.0 is required ' +
        'to synchronize with CalDAV servers.')

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
        if (args.init == 'keep-remote'):
            calcurse_wipe()
        elif (args.init == 'keep-local'):
            remote_wipe(conn)
        syncdb = {}
    else:
        # Read the synchronization database.
        syncdb = get_syncdb(syncdbfn)

        if not syncdb:
            die('Sync database not found or empty. Please initialize the ' +
                'database first.\n\nSupported initialization modes are:\n' +
                '  --init=keep-remote Remove all local calcurse items and import remote objects\n' +
                '  --init=keep-local  Remove all remote objects and push local calcurse items\n' +
                '  --init=two-way     Copy local objects to the CalDAV server and vice versa')

    # Query the server and build a dictionary that maps ETags to paths on the
    # server.
    hrefmap = get_hrefmap(conn)

    # Retrieve new objects from the server, delete local items that no longer
    # exist on the server.
    local_new, local_del = pull_objects(conn, syncdb, hrefmap)

    # Push new objects to the server, remove items from the server if they no
    # longer exist locally.
    remote_new, remote_del = push_objects(conn, syncdb, hrefmap)

    # Write the synchronization database.
    save_syncdb(syncdbfn, syncdb)

    # Close the HTTPs connection.
    conn.close()
finally:
    # Remove lock file.
    os.remove(lockfn)

# Print a summary to stdout.
print("%d items imported, %d items removed locally." % (local_new, local_del))
print("%d items exported, %d items removed from the server." %
      (remote_new, remote_del))
