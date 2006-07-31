#!/bin/sh
#
# Copyright (c) 2004-2006 Frederic Culot
#                                                                      
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#                                                                      
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#                                                                      
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#                                                                       
# Send your feedback or comments to : calcurse@culot.org
# Calcurse home page : http://culot.org/calcurse
#
#        $calcurse: autogen.sh,v 1.1 2006/07/31 21:00:02 culot Exp $
#
#
# autogen.sh - Generates all the necessary files to build calcurse from
# cvs tree.
#

PKG_NAME=calcurse
AC_VERSION="2 59"
AUTOCONF_VERSION=2.59
AC_FLAGS=
AM_VERSION="1 9"
AUTOMAKE_VERSION=1.9
AM_FLAGS="--gnu --copy --add-missing"
GETTEXT_VERSION="0 14"
GETTEXT_FLAGS="--copy --no-changelog"
ACLOCAL_FLAGS="-I m4"
SRCDIR=`dirname $0`
test -z "$SRCDIR" && SRCDIR=.
export AUTOMAKE_VERSION AUTOCONF_VERSION

# Function to check if we are at the top level of calcurse package.
check_directory_level()
{
        (test -f $SRCDIR/configure.ac) || {
                printf "\n\n**Error**: Directory "\`$SRCDIR\'" does not appear to"
                printf "\nbe the top-level $PKG_NAME directory.\n"
                exit 1
        }
}

# Clean previous files before running scripts
clean_old_files()
{
        printf "Cleaning old files ... "
        rm -rf configure config.log aclocal.m4 \
                config.status config autom4te.cache \
		po/Makefile.in.in
        printf "done\n"
}

# Clean useless backup files
clean_backup_files()
{
	printf "Cleaning backup files ... "
	rm -rf configure.ac\~ Makefile.am\~
	printf "done\n"
}

# Function to check for a program availability
check_program()
{
        PROGRAM=$1
        printf "Checking for $PROGRAM ... "
        ($PROGRAM --version) < /dev/null > /dev/null 2>&1 || {
                printf "\n\n**Error**: You must have $PROGRAM installed."
                printf "\nDownload the appropriate package for your distribution,"
                printf "\nor get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
		printf "\n"
                exit 1
        }
	FOUND=`which $PROGRAM`
	printf "$FOUND\n"
}

# Function to check a program's version
# (there must be a better way, but I am not good at sed...)
check_program_version()
{
        PROGRAM=$1; MAJOR=$2; MINOR=$3
        printf "Checking that $PROGRAM version is at least $MAJOR.$MINOR ... "
        VERSION=`$PROGRAM --version |head -n 1|sed 's/([^)]*)//g;s/^[a-zA-Z\.\ \
                \-]*//;s/ .*$//'`                
        MAJOR_FOUND=`echo $VERSION | cut -d. -f1`
        MINOR_FOUND=`echo $VERSION | sed s/[-,a-z,A-Z].*// | cut -d. -f2`
        [ -z "$MINOR_FOUND" ] && MINOR_FOUND=0
        
        WRONG=
        if [ -z "$MAJOR_FOUND" -lt "$MAJOR" ]; then
                WRONG=1
        elif [ "$MAJOR_FOUND" -eq "$MAJOR" ]; then
                if [ "$MINOR_FOUND" -lt "$MINOR" ]; then
                        WRONG=1
                fi
        fi        
        if [ ! -z "$WRONG" ]; then
                printf "\n\n**Error**: found version $MAJOR_FOUND.$MINOR_FOUND,"
                printf "\nwhich is too old. You should upgrade $PROGRAM."
                printf "\nDownload the appropriate package for your distribution,"
                printf "\nor get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
		printf "\n"
                exit 1
        else
                printf "OK, found $MAJOR_FOUND.$MINOR_FOUND\n"
        fi
}

# Dirty hack to run gettextize: problem is that it demands to
# press Return no matter what... This gets rid of that demand. 
run_gettext()
{
	PROGRAM=gettextize
	printf "Running $PROGRAM $GETTEXT_FLAGS ... "
	sed 's:read .*< /dev/tty::' `which $PROGRAM` > my-gettextize
	chmod +x my-gettextize
	(printf "\n" | ./my-gettextize $GETTEXT_FLAGS > /dev/null 2>&1) || {
		printf "\n\n**Error**: $PROGRAM failed.\n"
		exit 1
	}

	# now restore the files modified by gettextize 
	(test -f configure.ac~) && mv -f configure.ac~ configure.ac
	(test -f Makefile.am~)  && mv -f Makefile.am~  Makefile.am
	mv -f po/Makevars.template po/Makevars
	rm my-gettextize
	printf "OK\n"
}

# Function to run a program
run_program()
{
        PROGRAM=$1        
        shift
        PROGRAM_FLAGS=$@
        printf "Running $PROGRAM $PROGRAM_FLAGS ... "
        $PROGRAM $PROGRAM_FLAGS > /dev/null 2>&1 || {
                printf "\n\n**Error**: $PROGRAM failed.\n"
                exit 1
        }
	printf "OK\n"
}

# Main

echo " --- $PKG_NAME autogen script ---\n"
check_directory_level
clean_old_files
check_program gettext
check_program_version gettext $GETTEXT_VERSION
check_program aclocal
check_program autoheader
check_program automake
check_program_version automake $AM_VERSION
check_program autoconf
check_program_version autoconf $AC_VERSION
run_gettext 
run_program aclocal $ACLOCAL_FLAGS
run_program autoheader
run_program automake $AM_FLAGS
run_program autoconf $AC_FLAGS
clean_backup_files
printf "\nYou can now run the configure script to obtain $PKG_NAME Makefile.\n"
