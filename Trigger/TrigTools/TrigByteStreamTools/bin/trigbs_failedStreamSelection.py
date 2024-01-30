#!/usr/bin/env tdaq_python 

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Create an empty outfile if no events are selected for a given stream name in trigbs_extractStream.py

import sys

def empty_writer():
    """Creates an empty BSfile"""

    import eformat
    #retrieve input BSFile and corresponding runNumber 
    inputFile = sys.argv[1]
    runnumber = eformat.EventStorage.pickDataReader(inputFile).runNumber()
    #create the empty BSFile with the name: 'T0debug.runnumber.unknown_debug.unknown.RAW._lb0000._TRF._0001.data'
    of = eformat.EventStorage.RawFileName('T0debug',runnumber,'unknown','debug',0,'TRF','unknown')
    eformat.ostream(directory='.', core_name=of.fileNameCore())
    sys.exit(0)

if __name__ == "__main__":
    empty_writer()
