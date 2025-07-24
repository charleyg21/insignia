#!/bin/bash

BINDIR=.
MUMMER=mummer

if (( $# < 3 )); then
    echo "Usage: $0 OUT TGT BG1 [...BG32]"
    exit 1
fi

cov=$1; shift
tgt=$1; shift

if (( $# > 32 )); then
    echo "ERROR: No more than 32 background files possible"
    exit 1
fi
bgs=""
while (( $# > 0 )); do
    bgs="$bgs $1"; shift
done

$MUMMER -maxmatch -l 18 -n -b -c -F $tgt $bgs | $BINDIR/mcover > $cov
