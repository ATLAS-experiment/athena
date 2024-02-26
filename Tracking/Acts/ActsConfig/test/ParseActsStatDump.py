#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for dumping ACTS statistics after reco')
    parser.add_argument("--inputFile", help='Name of the input log file', required=True)
    return parser.parse_args()

def main(fileName: str):
    assert isinstance(fileName ,str)
    
    import re
    with open(fileName, 'r') as inFile:
        for line in inFile:
            # Check stat report statement
            match = re.findall('(Acts\S+\s+INFO.*statistics)', line)
            if match:
                print(match[0])

            match = re.findall('(Acts\S+\s+INFO.*Ratios)', line)
            if match:
                print(match[0])
                
            # print table
            match = re.findall('\d{2}:\d{2}:\d{2}\s(\|[A-Za-z0-9. +/-]+\|[^a-df-z]+\|$)', line)
            if match:
                print(match[0])
            
if __name__ == "__main__":
    # Parse the arguments
    MyArgs = GetCustomAthArgs()
    main(MyArgs.inputFile)
    
