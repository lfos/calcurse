#!/bin/sh

if [ "$1" = 'actual' ]; then
  calcurse -D data/ -t0 | sort -n
elif [ "$1" = 'expected' ]; then
  (
    echo 'completed tasks:'
    sed -n 's/^\[-\([0-9]\)\] \(.*\)/\1. \2/p' data/todo
  ) | sort -n
else
  ./run-test "$0"
fi
