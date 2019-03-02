calcurse-vdirsyncer
===============

calcurse-vdirsyncer is a Python script that can be used to synchronize
calcurse with a remote using [vdirsyncer](https://github.com/pimutils/vdirsyncer).
Please note that the script is alpha software!  This means that:

* We are eagerly looking for testers to run the script and give feedback! If
  you find any bugs, please report them to the calcurse mailing lists or to the
  GitHub bug tracker. If the script works fine for you, please report back as
  well!

* The script might still have bugs. MAKE BACKUPS, especially before running
  calcurse-vdirsyncer for the first time!

Usage
-----

calcurse-vdirsyncer requires an up-to-date version of calcurse and vdirsyncer.
Vdirsyncer needs to be properly configured before running the script.
See the [documentation](https://vdirsyncer.pimutils.org/en/stable/tutorial.html)
for a full configuration tutorial.

To run calcurse-vdirsyncer, call the script using

```sh
calcurse-vdirsyncer <vdir>
```

where `vdir` is the local storage directory specified in the vdirsyncer configuration file.

You can optionally specify an alternative directory for local calcurse data using the
`-D` flag if it differs from the default `~/.calcurse`.

How It Works
------------

calcurse-vdirsyncer leverages vdirsyncer to synchronize calcurse data between
different remotes. The script itself is a wrapper for calcurse import and export
commands and vdirsyncer synchronization calls.

When started, the script does the following

- Export all calcurse objects to the vdir directory
- Delete all local objects in the vdir directory not present in calcurse anymore
- Synchronize the vdir directory using vdirsyncer
- Import all new items in the vdir to calcurse
- Delete all calcurse items removed from the remote

Planned Updates
---------------

- Support for hook directories
- Enable filtering of imported and exported items (events, todos)
- Improve event parsing robustness
