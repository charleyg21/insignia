#!/bin/bash

set -e

# === CONFIG ===
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TESTDATA_DIR="$ROOT_DIR/test-data"
DBSEQ_DIR="$ROOT_DIR/db_seq"
PROGRAMS="$ROOT_DIR/programs"
CLEAN_SUFFIX=".clean.fna"
INS_SUFFIX=".ins.fna"
DBFILE="$DBSEQ_DIR/insignia.db.fna"
INDEXCSV="$DBSEQ_DIR/insigniaindex.csv"
VERSIONLOG="$DBSEQ_DIR/insignia.version.log"
BUILDLOG="$DBSEQ_DIR/insignia_build.log"

mkdir -p "$DBSEQ_DIR"

echo "[INFO] Cleaning FASTA files and merging multi-sequence files"
for fna in $TESTDATA_DIR/*.fna; do
    echo "[DEBUG] Processing: $fna"
    base=$(basename "$fna" .fna)
    clean="$TESTDATA_DIR/${base}${CLEAN_SUFFIX}"

    # Run fasta-clean via stdin
    cat "$fna" | "$PROGRAMS/fasta-clean" > "$clean"

    if [ ! -s "$clean" ]; then
        echo "[WARN] Skipping empty clean file for $fna"
        continue
    fi

    count=$(grep -c '^>' "$clean")
    
    if [ "$count" -gt 1 ]; then
        echo "[INFO] Merging multi-sequence $base"
        merged=$(mktemp)
        head=$(head -n 1 "$clean")
        echo "$head" > "$merged"
        awk 'NR > 1 { if ($0 ~ /^>/) next; else printf "%s", $0 } END { print "" }' "$clean" | \
            sed 's/NNNN*/NNNN/g' >> "$merged"
        mv "$merged" "$clean"
    fi

    # Use the real header for naming
    header=$(head -n 1 "$clean")
    accession=$(echo "$header" | cut -d' ' -f1 | sed 's/^>//')
    cp "$clean" "$DBSEQ_DIR/${accession}${INS_SUFFIX}"
    echo "[INFO] Created: $DBSEQ_DIR/${accession}${INS_SUFFIX}"
done

echo "[INFO] Creating combined FASTA database"
if compgen -G "$DBSEQ_DIR"/*.ins.fna > /dev/null; then
    cat "$DBSEQ_DIR"/*.ins.fna > "$DBFILE"
else
    echo "[WARN] No .ins.fna files found for database build."
    exit 1
fi

echo "[INFO] Building index CSV (accession + full header)"
: > "$INDEXCSV"
for f in "$DBSEQ_DIR"/*.ins.fna; do
    header=$(head -n 1 "$f")
    accession=$(echo "$header" | cut -d' ' -f1 | sed 's/^>//')
    echo -e "${accession}\t${header}" >> "$INDEXCSV"
done

echo "[INFO] Writing build log and version log"
timestamp=$(date '+%Y-%m-%d %H:%M:%S')
echo "Build completed at $timestamp" > "$BUILDLOG"
echo "InsigniaDB v1.0 built at $timestamp" > "$VERSIONLOG"

echo "✅ All database files successfully built."
echo "📁 Cleaned FASTAs: $TESTDATA_DIR/*$CLEAN_SUFFIX"
echo "📁 Sequence DB:    $DBSEQ_DIR/*.ins.fna + $DBFILE"
echo "📁 Index:          $INDEXCSV"
