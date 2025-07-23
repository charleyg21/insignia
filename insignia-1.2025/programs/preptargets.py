#!/usr/bin/env python3

import os

# Paths
idx_path = "../db_seq/t.idx"
proj_dir = "../projects"

# Ensure index exists
if not os.path.exists(idx_path):
    print("❌ Cannot find t.idx. Please run fasta-index first.")
    exit(1)

# Load 3-column index (assumes t.idx is valid)
entries = []
with open(idx_path, 'r') as f:
    for line in f:
        parts = line.strip().split('\t')
        if len(parts) >= 3:
            entries.append(parts[:3])

# List available IDs
print("🧬 Available target sequence IDs:\n")
ids = [entry[0] for entry in entries]
for i, seq_id in enumerate(ids, 1):
    print(f"{i:2d}. {seq_id}")

# Choose target
while True:
    try:
        selection = int(input("\n🔹 Enter the number of the target sequence you want to use: "))
        if 1 <= selection <= len(ids):
            target_id = ids[selection - 1]
            break
        else:
            print("❌ Invalid number.")
    except ValueError:
        print("❌ Please enter a number.")

print(f"\n✅ Selected target: {target_id}")

# Write output files
os.makedirs(proj_dir, exist_ok=True)
targets_path = os.path.join(proj_dir, "targets.txt")
backgrounds_path = os.path.join(proj_dir, "backgrounds.txt")

with open(targets_path, 'w') as tgt, open(backgrounds_path, 'w') as bg:
    for entry in entries:
        line = '\t'.join(entry) + '\n'
        if entry[0] == target_id:
            tgt.write(line)
        else:
            bg.write(line)

print(f"📁 Created {targets_path} and {backgrounds_path}")
