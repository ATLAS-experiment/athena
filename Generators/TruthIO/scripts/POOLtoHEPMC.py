#!/usr/bin/env python3

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Simple script for converting an EVNT, HITS, or RDO file into a HEPMC file

# Example execution:
# POOLtoHEPMC.py --filesInput=EVNT.34993204._006761.pool.root.1 Output.HepMCFileName=output.hepmc

# Options: input and output file, and compression (tgz)
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.addFlag('Output.HepMCFileName','events.hepmc',help='Name of the output HepMC file; files with .tgz, .gz, or .tar.gz extensions will be compressed')
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))

McEventKey = 'GEN_EVENT'
if 'McEventCollection#GEN_EVENT' not in flags.Input.TypedCollections:
    if 'McEventCollection#TruthEvent' in flags.Input.TypedCollections:
        McEventKey = 'TruthEvent'
    else:
        print('Truth collection not found in input file. Might be a problem.')

# We need the component factory to build the job up
from AthenaConfiguration.ComponentFactory import CompFactory

# Add FixHepMC to remove loops here
# This is a work-around for AGENE-2342, which needs a HepMC patch to fix
cfg.addEventAlgo(CompFactory.FixHepMC("FixHepMC"))

# Get the name of the uncompressed events file that we will write
events_filename = flags.Output.HepMCFileName.replace('.tgz','').replace('.tar','').replace('.gz','')

# Use the WriteHepMC AlgTool from TruthIO to do the conversion
cfg.addEventAlgo( CompFactory.WriteHepMC( 'WriteHepMC',
                  OutputFile = events_filename,
                  McEventKey = McEventKey ) )
cfg.run(flags.Exec.MaxEvents)

# Check based on the file name if we need to compress the output
if '.tgz' in flags.Output.HepMCFileName or '.tar.gz' in flags.Output.HepMCFileName:
    print('Compressing output into tar+gz format (this may take a moment)')
    import tarfile
    tar = tarfile.open(flags.Output.HepMCFileName,'w:gz')
    tar.add(events_filename)
    tar.close()
    # Remove the original uncompressed file
    import os
    os.remove(events_filename)
elif '.gz' in flags.Output.HepMCFileName:
    print('Compressing output into gz format (this may take a moment)')
    import gzip
    import shutil
    with open(events_filename,'rb') as in_file, gzip.open(flags.Output.HepMCFileName,'wb') as out_file:
        shutil.copyfileobj(in_file,out_file)
    # Remove the original uncompressed file
    import os
    os.remove(events_filename)
