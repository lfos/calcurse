calcurse-vdir
===============

calcurse-vdir is a Python script designed to export and import data to and
from directories following the
[vdir](http://vdirsyncer.pimutils.org/en/stable/vdir.html) storage format.
This data can then be synced with various remotes using tools like
[vdirsyncer](https://github.com/pimutils/vdirsyncer).
Please note that the script is alpha software! This means that:

* We are eagerly looking for testers to run the script and give feedback! If
  you find any bugs, please report them to the calcurse mailing lists or to the
  GitHub bug tracker. If the script works fine for you, please report back as
  well!

* The script might still have bugs. MAKE BACKUPS, especially before running
  calcurse-vdir with the `-f` flag!

Usage
-----

calcurse-vdir requires an up-to-date version of calcurse and python.
To run calcurse-vdir, call the script using

```sh
calcurse-vdir <action> <vdir>
```

where `action` is either `import` or `export` and where `vdir` is the local
directory to interact with.

When importing events, calcurse-vdir imports every event found in the vdir
directory that is not also present in calcurse. When exporting events,
calcurse-vdir does the opposite and writes any new event to the vdir directory.

These operations are non-destructive by default, meaning that no event will be
deleted by the script. The `-f` flag can be used to make the origin mirror the
destination, potentially deleting events in the destination that are no longer
present in the origin.

You can optionally specify an alternative directory for local calcurse data
using the `-D` flag if it differs from the default `$XDG_DATA_HOME/calcurse`
(`~/.local/share/calcurse`) or `~/.calcurse`.

Integration with vdirsyncer
---------------------------

A vdirsyncer synchronisation script `calcurse-vdirsyncer` is can be found in
the `contrib` directory. This script wraps event export, vdirsyncer
synchronization and imports in a single call. Run `calcurse-vdirsyncer -h` for
more information.

Planned Updates
---------------

* Support for hook directories
* Enable filtering of imported and exported items (events, todos)
* Improve event parsing robustness
* Add testing support
