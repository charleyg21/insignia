#!/bin/bash
set -e

ROOT=".."
TESTDATA="$ROOT/test-data"
DBSEQ="$ROOT/db_seq"
PROJECTS="$ROOT/projects"
DBCOV="$ROOT/db_cov"
PROGRAMS="$ROOT/programs"
DBFILE="$DBSEQ/insignia.db.fna"
INDEXCSV="$DBSEQ/insigniaindex.csv"
TARGETS="$PROJECTS/targets.txt"
BACKGROUNDS="$PROJECTS/backgrounds.txt"
TMPDIR=$(mktemp -d)
CLEAN_SUFFIX=".clean.fna"
INS_SUFFIX=".ins.fna"

mkdir -p "$DBSEQ" "$PROJECTS" "$DBCOV"

echo "[INFO] Scanning for new .fna files in $TESTDATA..."

for fna in "$TESTDATA"/*.fna; do
  base=$(basename "$fna" .fna)
  clean="$TESTDATA/${base}${CLEAN_SUFFIX}"

  # Skip if already processed
  if ls "$DBSEQ"/*.ins.fna | grep -q "${base}"; then
    echo "⏭️  Skipping $base — already exists."
    continue
  fi

  echo "[INFO] Processing new genome: $base"

  # Clean
  cat "$fna" | "$PROGRAMS/fasta-clean" > "$clean"
  if [ ! -s "$clean" ]; then
    echo "[WARN] Empty cleaned file for $fna — skipping."
    continue
  fi

  count=$(grep -c '^>' "$clean")
  if [ "$count" -gt 1 ]; then
    echo "[INFO] Merging multi-sequence: $base"
    head=$(head -n 1 "$clean")
    echo "$head" > "$TMPDIR/merged.fna"
    awk 'NR > 1 { if ($0 ~ /^>/) next; else printf "%s", $0 } END { print "" }' "$clean" >> "$TMPDIR/merged.fna"
    mv "$TMPDIR/merged.fna" "$clean"
  fi

  # Format headers
  header=$(head -n 1 "$clean")
  accession=$(echo "$header" | cut -d' ' -f1 | sed 's/^>//')
  species=$(echo "$header" | cut -d' ' -f2- | sed 's/ /_/g')
  new_header=">$accession $species"
  echo "$new_header" > "$DBSEQ/${accession}${INS_SUFFIX}"
  tail -n +2 "$clean" >> "$DBSEQ/${accession}${INS_SUFFIX}"

  echo -e "${accession}	${new_header}" >> "$INDEXCSV"
  cat "$DBSEQ/${accession}${INS_SUFFIX}" >> "$DBFILE"

  echo "[INFO] Created and added: $accession"

  # Run alignments to and from existing genomes
  for other in "$DBSEQ"/*.ins.fna; do
    other_id=$(basename "$other" .ins.fna)
    if [ "$other_id" != "$accession" ]; then
      mummer -maxmatch -l 15 -b -c -n -F "$DBSEQ/$accession.ins.fna" "$other" > "$PROJECTS/${accession}_vs_${other_id}.mums"
      ./mcover < "$PROJECTS/${accession}_vs_${other_id}.mums" > "$PROJECTS/${accession}_vs_${other_id}.match"

      mummer -maxmatch -l 15 -b -c -n -F "$other" "$DBSEQ/$accession.ins.fna" > "$PROJECTS/${other_id}_vs_${accession}.mums"
      ./mcover < "$PROJECTS/${other_id}_vs_${accession}.mums" > "$PROJECTS/${other_id}_vs_${accession}.match"
    fi
  done

  # Build new .cov file for this genome
  cat "$PROJECTS/${accession}_vs_"*.match > "$DBCOV/$accession.cov"
  echo "[INFO] Created .cov file: $DBCOV/$accession.cov"

  # Add to targets and backgrounds
  if ! grep -q "^$accession" "$TARGETS"; then
    echo -e "${accession}	${species}	0" >> "$TARGETS"
  fi
  if ! grep -q "^$accession" "$BACKGROUNDS"; then
    echo -e "${accession}	${species}	0" >> "$BACKGROUNDS"
  fi
done

# Rebuild t.idx
"$PROGRAMS/fasta-index" "$DBFILE" > "$DBSEQ/t.idx"
echo "✅ Genome addition complete. Updated index and database."
