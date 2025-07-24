#!/bin/bash

# Set variables
ROOT=".."
PROJECTS="$ROOT/projects"
PROGRAMS="$ROOT/programs"
TARGETS="$PROJECTS/targets.txt"
BACKGROUNDS="$PROJECTS/backgrounds.txt"

# Prompt for reference ID
echo "[?] Available targets:"
cut -f1 "$TARGETS"
echo -n "[?] Enter reference ID (must match ID in targets.txt): "
read REFID

# Check if REFID exists
if ! grep -q "^$REFID" "$TARGETS"; then
  echo "❌ Reference ID '$REFID' not found in targets.txt"
  exit 1
fi

# Define file paths
TGTFILE="$PROJECTS/${REFID}.tgt"
BGXFILE="$PROJECTS/${REFID}.bgx"
REFFILE="$PROJECTS/${REFID}.ref"
SHRFILE="$PROJECTS/${REFID}.shr"
UNIFILE="$PROJECTS/${REFID}.uni"
SIGFILE="$PROJECTS/${REFID}.sig"

# Build .tgt, .bgx, .ref
echo "[INFO] Creating .tgt, .bgx, .ref"
grep "^$REFID" "$TARGETS" > "$TGTFILE"
grep -v "^$REFID" "$BACKGROUNDS" > "$BGXFILE"
grep "^$REFID" "$BACKGROUNDS" > "$REFFILE"

# Generate shared cover
echo "[INFO] Generating shared k-mer match cover"
cat "$PROJECTS/${REFID}_vs_"*.match | \
"$PROGRAMS/mcover-intersect" -T "$TGTFILE" -B "$BGXFILE" > "$SHRFILE"

# Generate unique cover
echo "[INFO] Generating union of all matches"
cat "$PROJECTS/${REFID}_vs_"*.match | \
"$PROGRAMS/mcover-union" -T "$TGTFILE" > "$UNIFILE"

# Generate final .sig file
echo "[INFO] Generating final signature set"
cat "$UNIFILE" | "$PROGRAMS/unique-mer" -k 100 > "$SIGFILE"

# Done
echo "✅ Signature generation complete:"
echo " - Reference: $REFFILE"
echo " - Targets:   $TGTFILE"
echo " - Background: $BGXFILE"
echo " - Shared:    $SHRFILE"
echo " - Unique:    $UNIFILE"
echo " - Signatures: $SIGFILE"
