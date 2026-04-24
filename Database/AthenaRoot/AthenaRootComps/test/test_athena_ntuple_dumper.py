#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Utils import unixtools

# Set up command line arguments and flags
flags = initConfigFlags()
parser = flags.getArgumentParser()
parser.add_argument('--tupleName', default='egamma', help='TTree name')
parser.add_argument('--doWrite', type=int, choices=[0,1,2], default=1, help='Number of outputs to write')
args = flags.fillFromArgs(parser=parser)

if '_ATHENA_GENERIC_INPUTFILE_NAME_' in flags.Input.Files:
    paths = os.getenv('DATAPATH').split(os.pathsep) + [os.getenv('ATLAS_REFERENCE_DATA','')]
    testdir = unixtools.find_datafile(os.getenv('ATLAS_REFERENCE_TAG'), paths)
    flags.Input.Files = [os.path.join(testdir, f) for f in ('ntuple.0.root', 'ntuple.1.root')]

flags.lock()


# Create ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaRootComps.RootReadConfig import RootReadCfg
acc = MainServicesCfg(flags)
acc.merge( RootReadCfg(flags, tupleName=args.tupleName) )

# Add ASCII dumper
acc.addEventAlgo( CompFactory.Athena.RootAsciiDumperAlg("rootdumper") )

if args.doWrite:
    from AthenaRootComps.RootWriteConfig import NtupleOutputStreamCfg
    acc.addService(CompFactory.DecisionSvc())

    # Configure output stream
    acc.merge( NtupleOutputStreamCfg(flags,
                                     streamName = "StreamD3PD",
                                     fileName = "d3pd.root",
                                     tupleName = "egamma",
                                     ItemList = ["el_n", "el_eta", "el_jetcone_dr"],
                                     forceRead = True) )


    if args.doWrite==2:
        acc.merge( NtupleOutputStreamCfg(flags,
                                         streamName = "StreamD3PD_2",
                                         fileName = "d3pd_2.root",
                                         tupleName = "egamma",
                                         ItemList = ["el_n", "el_eta", "el_jetcone_dr"],
                                         forceRead = True) )

# Run
if __name__ == "__main__":
    sys.exit(acc.run().isFailure())
