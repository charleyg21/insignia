
# 🧬 Insignia Microbial Signature Pipeline (Command Line)

This pipeline builds a unique microbial signature database using whole-genome sequences. It performs genome cleaning, alignment (via MUMmer), match detection, and unique signature generation. All scripts run from the `programs/` directory using the test-data and db_seq folders as input and output points.

---

## 🔧 System Requirements & Dependencies

**OS:** Ubuntu, Linux, or macOS  
**Shell:** bash  
**Languages:** Python 3.8+

### Required Tools:
- MUMmer v3.x (`nucmer`, `show-coords`, `mummer`)
- BEDTools
- Python 3 standard library (`os`, `csv`, `subprocess`, `argparse`)
- Shell utilities: `cut`, `grep`, `awk`, `sed`, `sort`, `head`, `tail`, `basename`
- gcc or clang (for compiling C/C++ tools like mcover)

---

## 📁 Folder Structure

```
insignia/
├── programs/              # All pipeline scripts
├── test-data/             # Input .fna genome files (multi- or single-FASTA)
├── db_seq/                # Processed genomes, .ins.fna, .db.fna, and indexes
├── db_cov/                # Final .cov files (per target genome)
├── projects/              # Per-target alignment results (.mums, .match)
├── signatures/            # Final .sig files
```

---

## 📄 File Naming Requirements

- Input genome files must be placed in `test-data/`
- Must be named using GenBank accession format: `NC_009848.4.fna`
- Headers will be auto-cleaned to: `>ACCESSION Genus_species`  
  Example: `>NC_009848.4 Bacillus_pumilus`

---

## 🚀 How to Use the Pipeline

Run these commands from the `programs/` directory:

### 1. Create the database:
```bash
./createdb.sh
```
- Cleans headers
- Creates `.clean.fna`, `.ins.fna`, `insignia.db.fna`, `t.idx`, and `insigniaindex.csv`

### 2. Run alignments:
```bash
./buildalignments.sh
```
- Runs pairwise comparisons
- Outputs `.mums` and `.match` files to `projects/<target>/`

### 3. Generate signatures:
```bash
./signify.sh
```
- Lets you choose a target
- Outputs `.sig` and `.ref` files to `signatures/`

---

## 🧾 Output File Descriptions

- `*.clean.fna`: Cleaned FASTA files
- `*.ins.fna`: Indexed headers used in database
- `insignia.db.fna`: Combined reference database
- `t.idx`: FASTA byte offset index
- `*.mums`, `*.match`: Pairwise alignment results
- `*.cov`: Coverage files for each background
- `*.sig`: Signature output files

---

## ➕ Adding New Genomes

- Drop `.fna` files into `test-data/`
- Delete old `.clean.fna` if needed
- Re-run:
```bash
./createdb.sh
./buildalignments.sh
```

---

## 📌 Targets and Backgrounds

Auto-generated from `t.idx` by `createdb.sh`

- `targets.txt`: List of accession IDs (one per line)
- `backgrounds.txt`: ACCESSION START_BYTE GENOME_LENGTH

---

## 🧼 Tips for Clean Execution

- Run all scripts from `programs/`
- Use `git` to version control changes
- Avoid rerunning createdb.sh without deleting old `.clean.fna` files

---

## 🧪 Credits

Originally developed by NIH/Stanford. This is a modernized, scalable command-line reimplementation for local use.
