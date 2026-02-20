#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
Test to check output content of CPRun.py
"""

import argparse
import ROOT
import sys

def main():
    parser = argparse.ArgumentParser(description="Check for branches in a ROOT TTree.")
    parser.add_argument("--input_file", required=True, help="Path to the ROOT file")
    parser.add_argument("--tree_name", required=True, help="Name of the TTree inside the file")
    parser.add_argument("--branches", nargs="*", default=[], help="Branch names to check that should be present in tree")
    parser.add_argument("--forbidden-branches", nargs="*", default=[], help="Branch names to check that should not be present in tree")

    args = parser.parse_args()

    # Open ROOT file
    f = ROOT.TFile.Open(args.input_file)
    if not f or f.IsZombie():
        print(f"Error: Could not open file '{args.input_file}'")
        sys.exit(1)

    # Get tree
    tree = f.Get(args.tree_name)
    if not tree:
        print(f"Error: Could not find tree '{args.tree_name}' in file '{args.input_file}'")
        sys.exit(1)

    # Available branches
    available = {b.GetName() for b in tree.GetListOfBranches()}


    missing = False
    forbidden_found = False

    # Check required branches
    if args.branches:
        print()
        print(f"Checking required branches in tree '{args.tree_name}':")
        for br in args.branches:
            if br in available:
                print(f"[ OK ] {br}")
            else:
                print(f"[ MISSING ] {br}")
                missing = True

    # Check forbidden branches
    if args.forbidden_branches:
        print()
        print("Checking forbidden branches:")
        for br in args.forbidden_branches:
            if br in available:
                print(f"[ FORBIDDEN ] {br}")
                forbidden_found = True
            else:
                print(f"[ OK (not present) ] {br}")

    f.Close()

    # Exit non-zero if any issue found
    if missing or forbidden_found:
        sys.exit(2)

if __name__ == "__main__":
    main()
