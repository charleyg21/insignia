#!/bin/sh

BINDIR=.

if (( $# < 5 )); then
    echo "Usage: $0 DBPATH PREFIX MERLEN GBX REFID [TGTIDs ...]"
    exit 1
fi

opt_dbp=$1; shift
opt_pre=$1; shift
opt_len=$1; shift
opt_gbx=$1; shift
opt_ref=$1
opt_tgt=$@
opt_cov="${opt_dbp}/db_cov/${opt_ref}.cov"

out_ref="${opt_pre}.ref"
out_tgt="${opt_pre}.tgt"
out_bgx="${opt_pre}.bgx"
out_uni="${opt_pre}.uni"
out_shr="${opt_pre}.shr"
out_sig="${opt_pre}.sig"

rm -f $out_ref $out_tgt $out_bgx $out_uni $out_shr $out_sig

awk "{if(\$2==\"${opt_ref}\"){print}}" $opt_dbp/db_seq/t.idx > $out_ref

for tgt in $opt_tgt; do
    awk "{if(\$2==\"${tgt}\"){print}}" $opt_dbp/db_seq/t.idx >> $out_tgt
done

#-- Exclude Targets from Background
cp $out_tgt $out_bgx

#-- Exclude GenBank RefSeq from Background
if (( opt_gbx )); then
    echo "gi|*|	0	0" >> $out_bgx
fi

$BINDIR/mcover-union -T $out_ref -X $out_bgx $opt_cov | \
    $BINDIR/unique-mer -k $opt_len > $out_uni &

$BINDIR/mcover-intersect -T $out_ref -B $out_tgt $opt_cov | \
    $BINDIR/common-mer -k $opt_len > $out_shr &

wait

$BINDIR/kmer-intersect $out_uni $out_shr > $out_sig


#-- Clean up
rm -f $out_ref $out_tgt $out_bgx $out_uni $out_shr
