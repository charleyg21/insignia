#!/usr/bin/env python3

import os
import subprocess

# Paths
proj_dir = "../projects"
intersect_bin = "./mcover-intersect"  # must be in programs/
output_suffix = "_shared.match"

# Gather all .match files
match_files = sorted(f for f in os.listdir(proj_dir) if f.endswith(".match") and not f.startswith("."))

if not match_files:
    print("❌ No .match files found in ../projects/")
    exit(1)

# Parse target ID from filenames (assumes format: <target>_vs_<background>.match)
targets = set(f.split("_vs_")[0] for f in match_files)

if len(targets) != 1:
    print(f"❌ Expected all match files to use the same target. Found: {targets}")
    exit(1)

target = list(targets)[0]
output_path = os.path.join(proj_dir, f"{target}{output_suffix}")
input_paths = [os.path.join(proj_dir, f) for f in match_files]

print(f"🧬 Running mcover-intersect on {len(input_paths)} files for target: {target}")
print(f"▶ Output: {output_path}")

# Run intersect
with open(output_path, 'w') as out:
    subprocess.run([intersect_bin] + input_paths, stdout=out, check=True)

print("✅ Intersection complete.")

