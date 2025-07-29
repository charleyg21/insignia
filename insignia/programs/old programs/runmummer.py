#!/usr/bin/env python3

import os
import subprocess

# Set paths
db_dir = "../db_seq"
proj_dir = "../projects"

# Prompt user for target genome
print("🧬 Available genomes in db_seq/:")
genomes = [f for f in os.listdir(db_dir) if f.endswith(".ins.fna") and "insigniadb" not in f and not f.startswith(".")]
for g in genomes:
    print("   -", g)

target_file = input("\n🔹 Enter the filename of the target genome (e.g., NC_014551.1.ins.fna): ").strip()
target_path = os.path.join(db_dir, target_file)

if not os.path.exists(target_path):
    print(f"❌ Target file '{target_file}' not found in db_seq/. Aborting.")
    exit(1)

target_id = target_file.replace(".ins.fna", "")
print(f"\n✅ Target set: {target_id}\n")

# Make sure output dir exists
os.makedirs(proj_dir, exist_ok=True)

# Run self-match
self_output = os.path.join(proj_dir, f"{target_id}_vs_{target_id}.mums")
print(f"▶ Running MUMmer: {target_id} vs {target_id} (self-match)")
cmd = [
    "mummer",
    "-maxmatch", "-l", "15", "-b", "-c", "-n", "-F",
    target_path,
    target_path
]
with open(self_output, 'w') as out:
    subprocess.run(cmd, stdout=out, check=True)

# Run MUMmer for each background genome
for genome_file in genomes:
    if genome_file == target_file:
        continue

    bg_id = genome_file.replace(".ins.fna", "")
    bg_path = os.path.join(db_dir, genome_file)
    output_path = os.path.join(proj_dir, f"{target_id}_vs_{bg_id}.mums")

    print(f"▶ Running MUMmer: {target_id} vs {bg_id}")
    cmd = [
        "mummer",
        "-maxmatch", "-b", "-c", "-n", "-F",
        target_path,
        bg_path
    ]

    with open(output_path, 'w') as out:
        subprocess.run(cmd, stdout=out, check=True)

print("\n🎉 All alignments complete, including self-match.")

