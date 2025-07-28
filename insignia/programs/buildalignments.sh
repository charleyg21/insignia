#!/bin/bash

# Set directories
DBSEQ="../db_seq"
DBCOV="../db_cov"
PROJ="../projects"

# Sanity check
mkdir -p "$DBCOV"
mkdir -p "$PROJ"

# Read the list of .ins.fna files
INS_FILES=($(ls $DBSEQ/*.ins.fna))

echo "🔍 Checking for multi-sequence .ins.fna files and merging if needed..."

for FILE in "${INS_FILES[@]}"; do
  COUNT=$(grep -c '^>' "$FILE")
  if [ "$COUNT" -gt 1 ]; then
    echo "⚠️  $FILE has $COUNT sequences — merging with NNNN separator."
    HEADER=$(grep '^>' "$FILE" | head -n 1)
    OUTFILE=$(mktemp)
    echo "$HEADER" > "$OUTFILE"
    awk '/^>/ {if (seq) {print seq "NNNNNNNNNN"}; seq=""; next} {seq = seq $0} END {print seq}' "$FILE" >> "$OUTFILE"
    mv "$OUTFILE" "$FILE"
  fi
done

# Get list of genome IDs
GENOMES=($(ls $DBSEQ/*.ins.fna | xargs -n 1 basename | sed 's/.ins.fna//'))

echo "🔁 Generating MUMmer alignments and .match files for ${#GENOMES[@]} genomes..."

for TARGET in "${GENOMES[@]}"; do
  for COMP in "${GENOMES[@]}"; do
    MUMS_FILE="$PROJ/${TARGET}_vs_${COMP}.mums"
    MATCH_FILE="$PROJ/${TARGET}_vs_${COMP}.match"

    echo "▶️  MUMmer: $TARGET vs $COMP"
    mummer -maxmatch -l 15 -b -c -n -F "$DBSEQ/$TARGET.ins.fna" "$DBSEQ/$COMP.ins.fna" > "$MUMS_FILE"

    echo "▶️  mcover: $TARGET vs $COMP"
    ./mcover < "$MUMS_FILE" > "$MATCH_FILE"
  done

  echo "🧩 Creating .cov file for $TARGET"
  cat "$PROJ/${TARGET}_vs_"*.match > "$DBCOV/$TARGET.cov"
  echo "✅ $DBCOV/$TARGET.cov created"
done

echo "🎉 All alignments and .cov files complete."
