#!/bin/sh

CALCURSE=${CALCURSE:-../src/calcurse}
DATA_DIR=${DATA_DIR:-data/}

case "$CALCURSE" in
	/*) ;;
	*) CALCURSE="${PWD}/${CALCURSE}" ;;
esac
