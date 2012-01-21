#!/bin/sh

if [ "$1" = 'actual' ]; then
  calcurse -D data/ -t | sort
elif [ "$1" = 'expected' ]; then
  (
    echo 'to do:'
    sed '/^\[-/d; s/^\[\([0-9]\)\] \(.*\)/\1. \2/' data/todo
  ) | sort
else
  ./run-test "$0"
fi
