calcurse-caldav
===============

calcurse-caldav is a simple Python script that can be used to synchronize
calcurse with a CalDAV server. Please note that the script is alpha software!
This means that:

* We are eagerly looking for testers to run the script and give feedback! If
  you find any bugs, please report them to the calcurse mailing lists or to the
  GitHub bug tracker. If the script works fine for you, please report back as
  well!

* The script might still have bugs. MAKE BACKUPS, especially before running
  calcurse-caldav for the first time!

Usage
-----

calcurse-caldav requires an up-to-date version of calcurse and a configuration
file located at $XDG_CONFIG_HOME/calcurse/caldav/config
(~/.local/share/calcurse/caldav/config) or ~/.calcurse/caldav/config if
~/.calcurse exists. An example configuration file can be found under
contrib/caldav/config.sample in the calcurse source tree.  You will also need to
install *httplib2* for Python 3 using *pip* (e.g. `pip3 install --user
httplib2`) or your distribution's package manager.

If you run calcurse-caldav for the first time, you need to provide the `--init`
argument. You can choose between the following initialization modes:

    --init=keep-remote Remove all local calcurse items and import remote objects
    --init=keep-local  Remove all remote objects and push local calcurse items
    --init=two-way     Copy local objects to the CalDAV server and vice versa

For subsequent calcurse-caldav invocations, you don't need to specify any
additional parameters.

You can specify a username and password for basic authentication in the
config file. Alternatively, the password can be passed securely from another
program (such as *pass*) via the `CALCURSE_CALDAV_PASSWORD` environment variable like
so:
```
CALCURSE_CALDAV_PASSWORD=$(pass show calcurse) calcurse-caldav
```

Hooks
-----

You can place scripts in `$XDG_CONFIG_HOME/calcurse/caldav/hooks/`
(`~/.config/calcurse/caldav/hooks`) or `~/.calcurse/caldav/hooks` if
`~/.calcurse` exists in order to trigger actions at certain events. To enable a
hook, add a script with one of the following names to this directory. Also make
sure the scripts are executable.

*pre-sync*::
  Executed before the data files are synchronized.
*post-sync*::
  Executed after the data files are synchronized.

Some examples can be found in the `contrib/caldav/hooks/` directory of the
calcurse source tree.

How It Works
------------

calcurse-caldav creates a so-called synchronization database at
`$XDG_DATA_HOME/calcurse/caldav/sync.db`
(`~/.local/share/calcurse/caldav/sync.db`) or `~/.calcurse/caldav/sync.db` if
`~/.calcurse` exists that always keeps a snapshot of the last time the script
was executed. When running the script, it compares the objects on the server
and the local objects with that snapshot to identify items that were added or
deleted. It then

* downloads new objects from the server and imports them into calcurse,
* deletes local objects that no longer exist on the server,
* uploads objects to the server that were added locally,
* deleted objects from the server that were deleted locally,
* updates the synchronization database with a new snapshot.

Note: Since calcurse does not use unique identifiers for items, it cannot keep
track of moved/edited items. Thus, modifying an item is equivalent to deleting
the old item and creating a new one.

OAuth2 Authentication
---------------------

calcurse-caldav also has support for services requiring OAuth2 authentication
such as Google Calendar. Note that you can only have a single calendar synced
at any given time, regardless of authentication method. To enable OAuth2 you
will need to:

* Change *AuthMethod* from "*basic*" to "*oauth2*" in your config file
* Fill in the appropriate settings in your config file: "*ClientID*",
 "*ClientSecret*", "*Scope*", and "*RedirectURI*". These can be obtained from
 the API provider. (See below for Google Calendar)
* Install *oauth2client* for Python 3 using *pip* (e.g. `pip3 install --user
oauth2client`) or your distribution's package manager

Synchronization With Google Calendar
------------------------------------

You will need to use your Google account to create a Google API project and
enable both the CalDAV API and the Google Calendar API. We will be doing this to
receive a Client ID and Client Secret. The hostname, path, scope and redirect
URI are listed below.

First, you will need to go to the [Google Developers Console](https://console.developers.google.com/project) and click *Create
Project*. After doing that, you can go to the [API Library](https://console.developers.google.com/project/_/apiui/apis/library) and
search for the CalDAV API and enable it for your project. You will then need to
do the same for the Google Calendar API.

Next, go to the [Credentials page](https://console.developers.google.com/project/_/apiui/credential)
, click on *Create credentials*, and choose *OAuth client ID*. If it asks you
to "set a product name on the consent screen", click on *Configure consent
screen* to do so. Any product name will do. Upon saving and returning to the
"Create client ID" screen, choose *Other* as the Application type and click
*Create*. You now have your Client ID and Client Secret to put into your
calcurse-caldav config file!

The following options should also be changed in your config file:

```
Hostname = apidata.googleusercontent.com
Path = /caldav/v2/*your_calendar_id_here*/events/
Scope = https://www.googleapis.com/auth/calendar
SyncFilter = cal
```

Your Calendar ID for "*Path*" should be your email for the default calendar.
If you have multiple calendars, you can get the Calendar ID by going under
Calendar Details at [calendar.google.com](https://calendar.google.com).
The default Redirect URI in the config file is http://127.0.0.1; this should
work fine, but can be changed to http://localhost, a local web server, or
another device if you encounter errors related to it.

A complete config file for example@gmail.com would have the following options:

```
[General]

...

Hostname = apidata.googleusercontent.com
Path = /caldav/v2/example@gmail.com/events/
AuthMethod = oauth2
SyncFilter = cal

...

[OAuth2]
ClientID = 22802342jlkjlksjdlfkjq1htpvbcn.apps.googleusercontent.com
ClientSecret = XPYGqHFsfF343GwJeOGiUi
Scope = https://www.googleapis.com/auth/calendar
RedirectURI = http://127.0.0.1
```

[The full guide from Google can be found here.](https://developers.google.com/google-apps/calendar/caldav/v2/guide)

Upon your first run with the `--init` flag, you will be asked to go to a URL to
log in and authorize synchronization with your Google account. You can access
this URL on any other device if you cannot open a browser locally (e.g., on
a headless server). Once you authorize synchronization, you will be redirected
to your Redirect URI with a code embedded in it, e.g.,
`http://127.0.0.1/?code=4/Ok6mBNW2nppfIwyL-Q1ZPVkEk3zZdZN3mHcY#&scope=https://www.googleapis.com/auth/calendar`.
You will need to copy the code after `http://127.0.0.1/?code=` and before `&scope=https://www.googleapis.com/auth/calendar`. In this case,
it would be `4/Ok6mBNW2nppfIwyL-Q1ZPVkEk3zZdZN3mHcY#`.

Finally pass this authorization code to calcurse-caldav with the `--authcode`
flag and initialize the synchronization database like so (note that the quotes
around the authorization code might be necessary or not, depending on your shell):

```
calcurse-caldav --init keep-remote --authcode '4/Ok6mBNW2nppfIwyL-Q1ZPVkEk3zZdZN3mHcY#'
```

Troubleshooting
~~~~~~~~~~~~~~~

### 403 (Forbidden) When Submitting Auth Code for Google Calendar

- Ensure that both the CalDAV API and the Google Calendar API are enabled for
your Google Developer project
- Ensure that your authcode consists of only the `code` parameter's value when
extracting from the returned URL

Synchronization With Yahoo's Calendar Server
--------------------------------------------

The following configuration has been reported to work with Yahoo's calendar
server:

```
[General]
Binary = calcurse
Hostname = caldav.calendar.yahoo.com
Path = dav/*YOUR_USERNAME*/Calendar/*YOUR_CALENDAR_NAME*/
AuthMethod = basic
InsecureSSL = No

[Auth]
Username = *<username>*
Password = *<password>*

[CustomHeaders]
Scope = https://caldav.calendar.yahoo.com/
User-Agent = Mac_OS_X/10.15.1 (13C64) CalendarAgent/176
```
