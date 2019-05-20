#!/usr/bin/env python3

import argparse
import base64
import configparser
import httplib2
import os
import re
import subprocess
import sys
import textwrap
import urllib.parse
import xml.etree.ElementTree as etree

# Optional libraries for OAuth2 authentication
try:
    from oauth2client.client import OAuth2WebServerFlow, HttpAccessTokenRefreshError
    from oauth2client.file import Storage
    import webbrowser
except ModuleNotFoundError:
    pass


def msgfmt(msg, prefix=''):
    lines = []
    for line in msg.splitlines():
        lines += textwrap.wrap(line, 80 - len(prefix))
    return '\n'.join([prefix + line for line in lines])


def warn(msg):
    print(msgfmt(msg, "warning: "))


def die(msg):
    sys.exit(msgfmt(msg, "error: "))


def check_dir(dir):
    if not os.path.isdir(dir):
        die("invalid directory: {0}".format(dir))


def die_atnode(msg, node):
    if debug:
        msg += '\n\n'
        msg += 'The error occurred while processing the following XML node:\n'
        msg += etree.tostring(node).decode('utf-8')
    die(msg)


def validate_sync_filter():
    valid_sync_filter_values = {'event', 'apt', 'recur-event', 'recur-apt', 'todo', 'recur', 'cal'}
    return set(sync_filter.split(',')) - valid_sync_filter_values


def calcurse_wipe():
    if verbose:
        print('Removing all local calcurse objects...')
    if dry_run:
        return

    command = calcurse + ['-F', '--filter-hash=XXX']

    if debug:
        print('Running command: {}'.format(command))

    subprocess.call(command)


def calcurse_import(icaldata):
    command = calcurse + [
        '-i', '-',
        '--dump-imported',
        '-q',
        '--format-apt=%(hash)\\n',
        '--format-recur-apt=%(hash)\\n',
        '--format-event=%(hash)\\n',
        '--format-recur-event=%(hash)\\n',
        '--format-todo=%(hash)\\n'
    ]

    if debug:
        print('Running command: {}'.format(command))

    p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    return p.communicate(icaldata.encode('utf-8'))[0].decode('utf-8').rstrip()


def calcurse_export(objhash):
    command = calcurse + [
        '-xical',
        '--export-uid',
        '--filter-hash=' + objhash
    ]

    if debug:
        print('Running command: {}'.format(command))

    p = subprocess.Popen(command, stdout=subprocess.PIPE)
    return p.communicate()[0].decode('utf-8').rstrip()


def calcurse_hashset():
    command = calcurse + [
        '-G',
        '--filter-type', sync_filter,
        '--format-apt=%(hash)\\n',
        '--format-recur-apt=%(hash)\\n',
        '--format-event=%(hash)\\n',
        '--format-recur-event=%(hash)\\n',
        '--format-todo=%(hash)\\n'
    ]

    if debug:
        print('Running command: {}'.format(command))

    p = subprocess.Popen(command, stdout=subprocess.PIPE)
    return set(p.communicate()[0].decode('utf-8').rstrip().splitlines())


def calcurse_remove(objhash):
    command = calcurse + ['-F', '--filter-hash=!' + objhash]

    if debug:
        print('Running command: {}'.format(command))

    subprocess.call(command)


def calcurse_version():
    command = calcurse + ['--version']

    if debug:
        print('Running command: {}'.format(command))

    p = subprocess.Popen(command, stdout=subprocess.PIPE)
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


def init_auth(client_id, client_secret, scope, redirect_uri, authcode):
    # Create OAuth2 session
    oauth2_client = OAuth2WebServerFlow(client_id=client_id,
                                        client_secret=client_secret,
                                        scope=scope,
                                        redirect_uri=redirect_uri)

    # If auth code is missing, tell user run script with auth code
    if not authcode:
        # Generate and open URL for user to authorize
        auth_uri = oauth2_client.step1_get_authorize_url()
        webbrowser.open(auth_uri)

        prompt = ('\nIf a browser window did not open, go to the URL '
                  'below and log in to authorize syncing. '
                  'Once authorized, pass the string after "code=" from '
                  'the URL in your browser\'s address bar to '
                  'calcurse-caldav.py using the "--authcode" flag. '
                  "Example: calcurse-caldav --authcode "
                  "'your_auth_code_here'\n\n{}\n".format(auth_uri))
        print(prompt)
        die("Access token is missing or refresh token is expired.")

    # Create and return Credential object from auth code
    credentials = oauth2_client.step2_exchange(authcode)

    # Setup storage file and store credentials
    storage = Storage(oauth_file)
    credentials.set_store(storage)
    storage.put(credentials)

    return credentials


def run_auth(authcode):
    # Check if credentials file exists
    if os.path.isfile(oauth_file):

        # Retrieve token from file
        storage = Storage(oauth_file)
        credentials = storage.get()

        # Set file to store it in for future functions
        credentials.set_store(storage)

        # Refresh the access token if it is expired
        if credentials.invalid:
            try:
                credentials.refresh(httplib2.Http())
            except HttpAccessTokenRefreshError:
                # Initialize OAuth2 again if refresh token becomes invalid
                credentials = init_auth(client_id, client_secret, scope, redirect_uri, authcode)
    else:
        # Initialize OAuth2 credentials
        credentials = init_auth(client_id, client_secret, scope, redirect_uri, authcode)

    return credentials


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
        headers_sanitized = headers.copy()
        if not debug_raw:
            headers_sanitized.pop('Authorization', None)
        print("> Headers: " + repr(headers_sanitized))
        if body:
            for line in body.splitlines():
                print("> " + line)
        print()

    if isinstance(body, str):
        body = body.encode('utf-8')

    resp, body = conn.request(path, cmd, body=body, headers=headers)
    body = body.decode('utf-8')

    if not resp:
        return (None, None)

    if debug:
        print("< Status: {} ({})".format(resp.status, resp.reason))
        print("< Headers: " + repr(resp))
        for line in body.splitlines():
            print("< " + line)
        print()

    if resp.status - (resp.status % 100) != 200:
        die(("The server at {} replied with HTTP status code {} ({}) " +
             "while trying to access {}.").format(hostname, resp.status,
                                                  resp.reason, path))

    return (resp, body)


def get_etags(conn, hrefs=[]):
    if len(hrefs) > 0:
        headers = {}
        body = ('<?xml version="1.0" encoding="utf-8" ?>'
                '<C:calendar-multiget xmlns:D="DAV:" '
                '                     xmlns:C="urn:ietf:params:xml:ns:caldav">'
                '<D:prop><D:getetag /></D:prop>')
        for href in hrefs:
            body += '<D:href>{}</D:href>'.format(href)
        body += '</C:calendar-multiget>'
    else:
        headers = {'Depth': '1'}
        body = ('<?xml version="1.0" encoding="utf-8" ?>'
                '<C:calendar-query xmlns:D="DAV:" '
                '                  xmlns:C="urn:ietf:params:xml:ns:caldav">'
                '<D:prop><D:getetag /></D:prop>'
                '<C:filter><C:comp-filter name="VCALENDAR" /></C:filter>'
                '</C:calendar-query>')
    headers, body = remote_query(conn, "REPORT", absolute_uri, headers, body)
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

    remote_items = get_etags(conn)
    for href in remote_items:
        remove_remote_object(conn, remote_items[href], href)


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


def syncdb_add(syncdb, href, etag, objhash):
    syncdb[href] = (etag, objhash)
    if debug:
        print('New sync database entry: {} {} {}'.format(href, etag, objhash))


def syncdb_remove(syncdb, href):
    syncdb.pop(href, None)
    if debug:
        print('Removing sync database entry: {}'.format(href))


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
    headers, body = remote_query(conn, "PUT", hostname_uri + href, {}, body)

    if not headers:
        return None

    etag = None
    headerdict = dict(headers)
    if 'etag' in headerdict:
        etag = headerdict['etag']
    while not etag:
        etagdict = get_etags(conn, [href])
        if etagdict:
            etag = next(iter(etagdict.values()))
    etag = etag.strip('"')

    return (urllib.parse.quote(href), etag)


def push_objects(objhashes, conn, syncdb, etagdict):
    # Copy new objects to the server.
    added = 0
    for objhash in objhashes:
        if verbose:
            print("Pushing new object {} to the server.".format(objhash))
        if dry_run:
            continue

        href, etag = push_object(conn, objhash)
        syncdb_add(syncdb, href, etag, objhash)
        added += 1

    return added


def remove_remote_object(conn, etag, href):
    headers = {'If-Match': '"' + etag + '"'}
    remote_query(conn, "DELETE", hostname_uri + href, headers, None)


def remove_remote_objects(objhashes, conn, syncdb, etagdict):
    # Remove locally deleted objects from the server.
    deleted = 0
    for objhash in objhashes:
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
                syncdb_remove(syncdb, href)
                continue

            if verbose:
                print("Removing remote object {} ({}).".format(etag, href))
            if dry_run:
                continue

            remove_remote_object(conn, etag, href)
            syncdb_remove(syncdb, href)
            deleted += 1

    return deleted


def pull_objects(hrefs_missing, hrefs_modified, conn, syncdb, etagdict):
    if not hrefs_missing and not hrefs_modified:
        return 0

    # Download and import new objects from the server.
    body = ('<?xml version="1.0" encoding="utf-8" ?>'
            '<C:calendar-multiget xmlns:D="DAV:" '
            '                     xmlns:C="urn:ietf:params:xml:ns:caldav">'
            '<D:prop><D:getetag /><C:calendar-data /></D:prop>')
    for href in (hrefs_missing | hrefs_modified):
        body += '<D:href>{}</D:href>'.format(href)
    body += '</C:calendar-multiget>'
    headers, body = remote_query(conn, "REPORT", absolute_uri, {}, body)

    root = etree.fromstring(body)

    added = 0

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

        if href in hrefs_modified:
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

        # TODO: Add support for importing multiple events at once, see GitHub
        # issue #20 for details.
        if re.match(r'[0-ga-f]+$', objhash):
            syncdb_add(syncdb, href, etag, objhash)
            added += 1
        else:
            print("Failed to import object: {} ({})".format(etag, href),
                  file=sys.stderr)

    return added


def remove_local_objects(hrefs, conn, syncdb, etagdict):
    # Delete objects that no longer exist on the server.
    deleted = 0
    for href in hrefs:
        etag, objhash = syncdb[href]

        if verbose:
            print("Removing local object {}.".format(objhash))
        if dry_run:
            continue

        calcurse_remove(objhash)
        syncdb_remove(syncdb, href)
        deleted += 1

    return deleted


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
oauth_file = os.path.expanduser("~/.calcurse/caldav/oauth2_cred")

# Parse command line arguments.
parser = argparse.ArgumentParser('calcurse-caldav')
parser.add_argument('--init', action='store', dest='init', default=None,
                    choices=['keep-remote', 'keep-local', 'two-way'],
                    help='initialize the sync database')
parser.add_argument('--config', action='store', dest='configfn',
                    default=configfn,
                    help='path to the calcurse-caldav configuration')
parser.add_argument('--datadir', action='store', dest='datadir',
                    default=None,
                    help='path to the calcurse data directory')
parser.add_argument('--lockfile', action='store', dest='lockfn',
                    default=lockfn,
                    help='path to the calcurse-caldav lock file')
parser.add_argument('--syncdb', action='store', dest='syncdbfn',
                    default=syncdbfn,
                    help='path to the calcurse-caldav sync DB')
parser.add_argument('--hookdir', action='store', dest='hookdir',
                    default=hookdir,
                    help='path to the calcurse-caldav hooks directory')
parser.add_argument('--authcode', action='store', dest='authcode',
                    default=None,
                    help='auth code for OAuth2 authentication')
parser.add_argument('-v', '--verbose', action='store_true', dest='verbose',
                    default=False,
                    help='print status messages to stdout')
parser.add_argument('--debug', action='store_true', dest='debug',
                    default=False, help='print debug messages to stdout')
parser.add_argument('--debug-raw', action='store_true', dest='debug_raw',
                    default=False, help='do not sanitize debug messages')
args = parser.parse_args()

init = args.init is not None
configfn = args.configfn
lockfn = args.lockfn
syncdbfn = args.syncdbfn
datadir = args.datadir
hookdir = args.hookdir
authcode = args.authcode
verbose = args.verbose
debug = args.debug
debug_raw = args.debug_raw

# Read environment variables
password = os.getenv('CALCURSE_CALDAV_PASSWORD')

# Read configuration.
config = configparser.RawConfigParser()
if verbose:
    print('Loading configuration from ' + configfn + '...')
try:
    config.read_file(open(configfn))
except FileNotFoundError as e:
    die('Configuration file not found: {}'.format(configfn))

if config.has_option('General', 'InsecureSSL'):
    insecure_ssl = config.getboolean('General', 'InsecureSSL')
else:
    insecure_ssl = False

# Read config for "HTTPS" option (default=True)
if config.has_option('General', 'HTTPS'):
    https = config.getboolean('General', 'HTTPS')
else:
    https = True

if config.has_option('General', 'Binary'):
    calcurse = [config.get('General', 'Binary')]
else:
    calcurse = ['calcurse']

if datadir:
    check_dir(datadir)
    calcurse += ['-D', datadir]

if config.has_option('General', 'DryRun'):
    dry_run = config.getboolean('General', 'DryRun')
else:
    dry_run = True

if not verbose and config.has_option('General', 'Verbose'):
    verbose = config.getboolean('General', 'Verbose')

if not debug and config.has_option('General', 'Debug'):
    debug = config.getboolean('General', 'Debug')

if config.has_option('General', 'AuthMethod'):
    authmethod = config.get('General', 'AuthMethod').lower()
else:
    authmethod = 'basic'

if config.has_option('General', 'SyncFilter'):
    sync_filter = config.get('General', 'SyncFilter')

    invalid_filter_values = validate_sync_filter()

    if len(invalid_filter_values):
        die('Invalid value(s) in SyncFilter option: ' + ', '.join(invalid_filter_values))
else:
    sync_filter = 'cal,todo'

if config.has_option('Auth', 'UserName'):
    username = config.get('Auth', 'UserName')
else:
    username = None

if config.has_option('Auth', 'Password') and not password:
    password = config.get('Auth', 'Password')

if config.has_section('CustomHeaders'):
    custom_headers = dict(config.items('CustomHeaders'))
else:
    custom_headers = {}

if config.has_option('OAuth2', 'ClientID'):
    client_id = config.get('OAuth2', 'ClientID')
else:
    client_id = None

if config.has_option('OAuth2', 'ClientSecret'):
    client_secret = config.get('OAuth2', 'ClientSecret')
else:
    client_secret = None

if config.has_option('OAuth2', 'Scope'):
   scope = config.get('OAuth2', 'Scope')
else:
   scope = None

if config.has_option('OAuth2', 'RedirectURI'):
    redirect_uri = config.get('OAuth2', 'RedirectURI')
else:
    redirect_uri = 'http://127.0.0.1'

# Change URl prefix according to HTTP/HTTPS
if https:
    urlprefix = "https://"
else:
    urlprefix = "http://"

hostname = config.get('General', 'HostName')
path = '/' + config.get('General', 'Path').strip('/') + '/'
hostname_uri = urlprefix + hostname
absolute_uri = hostname_uri + path

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
    # Connect to the server.
    if verbose:
        print('Connecting to ' + hostname + '...')
    conn = httplib2.Http()
    if insecure_ssl:
        conn.disable_ssl_certificate_validation = True

    if authmethod == 'oauth2':
        # Authenticate with OAuth2 and authorize HTTP object
        cred = run_auth(authcode)
        conn = cred.authorize(conn)
    elif authmethod == 'basic':
        # Add credentials to httplib2
        conn.add_credentials(username, password)
    else:
        die('Invalid option for AuthMethod in config file. Use "basic" or "oauth2"')

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

    # Query the server and compute a lookup table that maps each path to its
    # current ETag.
    etagdict = get_etags(conn)

    # Compute object diffs.
    missing = set()
    modified = set()
    for href in set(etagdict.keys()):
        if href not in syncdb:
            missing.add(href)
        elif etagdict[href] != syncdb[href][0]:
            modified.add(href)
    orphan = set(syncdb.keys()) - set(etagdict.keys())

    objhashes = calcurse_hashset()
    new = objhashes - set([entry[1] for entry in syncdb.values()])
    gone = set([entry[1] for entry in syncdb.values()]) - objhashes

    # Retrieve new objects from the server.
    local_new = pull_objects(missing, modified, conn, syncdb, etagdict)

    # Delete local items that no longer exist on the server.
    local_del = remove_local_objects(orphan, conn, syncdb, etagdict)

    # Push new objects to the server.
    remote_new = push_objects(new, conn, syncdb, etagdict)

    # Remove items from the server if they no longer exist locally.
    remote_del = remove_remote_objects(gone, conn, syncdb, etagdict)

    # Write the synchronization database.
    save_syncdb(syncdbfn, syncdb)

    #Clear OAuth2 credentials if used
    if authmethod == 'oauth2':
        conn.clear_credentials()

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
