calcurse
========

Building
--------

If you are using a release tarball, the following commands can be used to build
and install calcurse:

    $ ./configure
    $ make
    $ make install

Note that `make install` needs to be run as root. When working on a Git
checkout, you need to run `./autogen.sh` before `./configure`.

Package Overview
----------------

* `src`: contains calcurse sources
* `test`: contains a test suite and test cases for calcurse
* `scripts`: contains additional scripts, such as `calcurse-upgrade`
* `doc`: contains detailed documentation in plain text and HTML

Authors
-------

* Lukas Fleischer (Maintainer)
* Frederic Culot (Original Author)

Contributors
------------

* RegEx support: Erik Saule
* Dutch translation: Jeremy Roon, 2007-2010
* French translation: Frédéric Culot, 2006-2010
* French translation: Toucouch, 2007
* French translation: Erik Saule, 2011-2012
* French translation: Stéphane Aulery, 2012
* French translation: Baptiste Jonglez, 2012
* German translation: Michael Schulz, 2006-2010
* German translation: Chris M., 2006
* German translation: Benjamin Moeller, 2010
* German translation: Lukas Fleischer, 2011-2012
* Portuguese (Brazil) translation: Rafael Ferreira, 2012
* Russian translation: Aleksey Mechonoshin, 2011-2012
* Spanish translation: Jose Lopez, 2006-2010

Also check the `Thanks` section in the manual for a list of people who have
contributed by reporting bugs, sending fixes, or suggesting improvements.
