#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Utility for generating model names for Triton

Input is either a single entry, or (with -l/--list) an ASCI file containing
one entry per line. Each entry is a path to the .onnx file relative to
the CALIBPATH environment. The output of the utility is a dictionary of
{Entry:ModelName} which is printed to stdout (default), text file, or json file

Usage examples:
    getTritonModelName.py BTagging/20250527/GN3V01/antikt4empflow/network.onnx
    getTritonModelName.py -l files.txt -o out.txt
    getTritonModelName.py -l files.txt --json -o out.json
"""

from PyUtils.TritonUtils import tritonModelName

import argparse
import json
import sys


def read_file_list(list_path):
    """Return file names from a list file, one per line (blank lines and # comments skipped)."""
    try:
        with open(list_path) as f:
            return [ln.strip() for ln in f if ln.strip() and not ln.lstrip().startswith("#")]
    except OSError as e:
        sys.exit(f"Error: cannot read list file '{list_path}': {e}")


def main():
    # Command-line interface
    p = argparse.ArgumentParser(description="Generate Triton model names for .onnx files")
    src = p.add_mutually_exclusive_group(required=True)
    src.add_argument("file", nargs="?", help="single input file")
    src.add_argument("-l", "--list", metavar="LISTFILE",
                     help="text file with one .onnx file entry per line")
    fmt = p.add_mutually_exclusive_group()
    fmt.add_argument("--text", dest="fmt", action="store_const", const="text",
                     help="write a plain text file, 'entry: name' per line (default)")
    fmt.add_argument("--json", dest="fmt", action="store_const", const="json",
                     help="write a JSON file")
    p.set_defaults(fmt="text")
    p.add_argument("-o", "--output", help="output file (default: stdout)")
    args = p.parse_args()

    # Collect inputs
    entries = read_file_list(args.list) if args.list else [args.file]

    # Generate model names for Triton
    result = {}
    for entry in entries:
        name = tritonModelName(entry)
        result[entry] = name or "N/A"

    # Write output
    if args.fmt == "json":
        text = json.dumps(result, indent=2) + "\n"
    else:
        text = "".join(f"{k}: {v}\n" for k, v in result.items())

    if args.output:
        with open(args.output, "w") as f:
            f.write(text)
    else:
        sys.stdout.write(text)

    return 1


if __name__ == "__main__":
    sys.exit(main())
