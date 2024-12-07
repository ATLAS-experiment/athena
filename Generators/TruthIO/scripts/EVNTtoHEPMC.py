#!/usr/bin/env python3

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Simple script for converting an EVNT file into a HEPMC file

# Options: input and output file, and compression (tgz)
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.addFlag('Output.HepMCFileName','events.hepmc',help='Name of the output HepMC file')
flags.addFlag('Output.CompressHepMC',False,help='Compress the output after Athena finishes')
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))

# Use the WriteHepMC AlgTool from TruthIO to do the conversion
from AthenaConfiguration.ComponentFactory import CompFactory
cfg.addEventAlgo( CompFactory.WriteHepMC( 'WriteHepMC',
                  OutputFile = flags.Output.HepMCFileName.replace('.tgz','') ) )

cfg.run(flags.Exec.MaxEvents)

# In case we were asked to, compress the output
if flags.Output.CompressHepMC:
    print('Compressing output (this may take a moment)')
    import tarfile
    final_name = flags.Output.HepMCFileName if '.tgz' in flags.Output.HepMCFileName else flags.Output.HepMCFileName+'.tgz'
    tar = tarfile.open(final_name,'w:gz')
    tar.add(flags.Output.HepMCFileName.replace('.tgz',''))
    tar.close()
    # Remove the original uncompressed file
    import os
    os.remove(flags.Output.HepMCFileName.replace('.tgz',''))
