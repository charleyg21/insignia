#!/usr/bin/env python3

import os
import subprocess

# Config
proj_dir = "../projects"
k = "100"
target = "NZ_CM000753.1"

# Input files
shared_match = os.path.join(proj_dir, f"{target}_shared.match")
targets_txt = os.path.join(proj_dir, "targets.txt")
backgrounds_txt = os.path.join(proj_dir, "backgrounds.txt")
match_files = [os.path.join(proj_dir, f) for f in os.listdir(proj_dir)
               if f.endswith(".match") and f.startswith(f"{target}_vs_")]

# Output files
shared_kmer = os.path.join(proj_dir, f"{target}_shared.kmer")
unique_kmer = os.path.join(proj_dir, f"{target}_unique.kmer")
signature_kmer = os.path.join(proj_dir, f"{target}_signature.kmer")

print("▶ Step 1: Generating shared k-mers with common-mer...")
with open(shared_match, 'r') as sin, open(shared_kmer, 'w') as sout:
    subprocess.run(["./common-mer", "-k", k], stdin=sin, stdout=sout, check=True)

print("▶ Step 2: Generating unique k-mers with mcover-merge + unique-mer...")
merge_cmd = ["./mcover-union", "-T", targets_txt] + match_files
merge_proc = subprocess.Popen(merge_cmd, stdout=subprocess.PIPE)

with open(unique_kmer, 'w') as uout:
    subprocess.run(["./unique-mer", "-k", k], stdin=merge_proc.stdout, stdout=uout, check=True)

print("▶ Step 3: Intersecting shared and unique k-mers with kmer-intersect...")
with open(signature_kmer, 'w') as sigout:
    subprocess.run(["./kmer-intersect", shared_kmer, unique_kmer], stdout=sigout, check=True)

print(f"\n✅ Final signature set written to: {signature_kmer}")

