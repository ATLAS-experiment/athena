#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This script converts the geometry.dat file output obtained from the run of RunPrintSiDetElements.py into a JSON file.
import json
import argparse
   
data = {}

parser = argparse.ArgumentParser(description="Converter from geometry dat file to json")
parser.add_argument(
    '--infile', 
    type=str, 
    help="Input .dat file",
    default="geometry.dat"
)
parser.add_argument(
    "--outfile",
    help="Output .json file",
    type=str,
    default="geometry.json",
)

args = parser.parse_args()


with open(args.infile) as f:
    for l in f:
            
        if l.startswith('#'):
            continue
            
        bec, ld, phi, eta, side, ID = l.split()[2:8]
        data[ID] = {"BEC" : bec, "LayerDisk" : ld, "PhiModule" : phi, "EtaModule" : eta, "Side" : side }

with open(args.outfile, "w") as f:
    json.dump(data, f, indent = 4)
