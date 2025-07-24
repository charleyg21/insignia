#!/usr/bin/bash

BINDIR=.
OUTDIR=/scratch2/insignia

if (( $# != 3 )); then
    echo "USAGE: $0 TG BG MACHINES"
    exit 1
fi

tg=$1
bg=$2
mach=$3

for ((i=0;;i++)); do
    if ! [ -e $tg.$i ]; then
        break;
    fi

    b=0

    #-- 1 target files vs 32 target files
    c=0
    tgs=""
    for ((j=0;;j++)); do

        if [ -e $tg.$j ]; then
            tgs="$tgs $tg.$j"
            ((c++))
        elif [ "$tgs" ]; then
            c=32
        fi

        if (( c >= 32 )); then

            echo "Launching $i.$b"
            runCmd \
                -o $i.$b.out \
                -e $i.$b.err \
                --nowait \
                --machines=$mach \
                -- $BINDIR/mcover-build $OUTDIR/$i.$b.cov $tg.$i $tgs \
                &> $i.$b.cmd &
            sleep 2

            ((b++))
            c=0
            tgs=""
        fi

        if ! [ -e $tg.$j ]; then
            break
        fi
    done


    #-- 1 target file vs 32 background files
    c=0
    bgs=""
    for ((j=0;;j++)); do

        if [ -e $bg.$j ]; then
            bgs="$bgs $bg.$j"
            ((c++))
        elif [ "$bgs" ]; then
            c=32
        fi

        if (( c >= 32 )); then

            echo "Launching $i.$b"
            runCmd \
                -o $i.$b.out \
                -e $i.$b.err \
                --nowait \
                --machines=$mach \
                -- $BINDIR/mcover-build $OUTDIR/$i.$b.cov $tg.$i $bgs \
                &> $i.$b.cmd &
            sleep 2

            ((b++))
            c=0
            bgs=""
        fi

        if ! [ -e $bg.$j ]; then
            break
        fi
    done
done
