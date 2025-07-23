#!/usr/bin/env python3

import os
import subprocess

# Paths relative to programs/
proj_dir = "../projects"
mcover_path = "./mcover"  # assumes mcover binary is here

# Collect all .mums files
mums_files = sorted(f for f in os.listdir(proj_dir) if f.endswith(".mums") and not f.startswith("."))

if not mums_files:
    print("❌ No .mums files found in ../projects/")
    exit(1)

print("🧬 Found MUMmer files:")
for f in mums_files:
    print("   -", f)

print("\n▶ Running mcover...")

for mums_file in mums_files:
    input_path = os.path.join(proj_dir, mums_file)
    match_file = mums_file.replace(".mums", ".match")
    output_path = os.path.join(proj_dir, match_file)

    with open(input_path, 'r') as fin, open(output_path, 'w') as fout:
        subprocess.run([mcover_path], stdin=fin, stdout=fout, check=True)

    print(f"✅ Created {match_file}")

print("\n🎉 All match files generated.")

