#!/bin/sh

if [ "$1" = 'actual' ]; then
  calcurse -D data/ -t3
elif [ "$1" = 'expected' ]; then
  echo 'to do:'
  sed -n 's/^\[3\] \(.*\)/3. \1/p' data/todo
else
  ./run-test "$0"
fi
