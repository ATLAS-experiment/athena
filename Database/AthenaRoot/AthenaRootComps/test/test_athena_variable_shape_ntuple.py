#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Utils import unixtools
from AthenaPython import PyAthena

StatusCode = PyAthena.StatusCode

# Set up command line arguments and flags
flags = initConfigFlags()
parser = flags.getArgumentParser()
parser.add_argument('--branches', default='', help='Branches to read')
parser.add_argument('--outbranches', default="el_n,el_eta,el_jetcone_dr", help='Branches to write')

args = flags.fillFromArgs(parser=parser)

if '_ATHENA_GENERIC_INPUTFILE_NAME_' in flags.Input.Files:
    paths = os.getenv('DATAPATH').split(os.pathsep) + [os.getenv('ATLAS_REFERENCE_DATA','')]
    testdir = unixtools.find_datafile(os.getenv('ATLAS_REFERENCE_TAG'), paths)
    flags.Input.Files = [os.path.join(testdir, f) for f in ('ntuple.0.root', 'ntuple.1.root')]

flags.lock()

class MyAlg( PyAthena.Alg ):

    def __init__(self, name='MyAlg', **kw):
        kw['name'] = name
        self.activeBranches = kw.get(
            'activeBranches',
            ['RunNumber',
             'EventNumber',
             'el_n',
             'el_eta',
             'el_jetcone_dr',
             ])
        self.fname = kw.get('fname', 'data.var.txt')
        super(MyAlg, self).__init__(**kw)
        return

    def initialize(self):
        self.evtstore = PyAthena.py_svc('StoreGateSvc')
        self.fd = open(self.fname, 'w')
        return StatusCode.Success

    def execute(self):
        self.msg.info('running execute...')
        keys = []
        for p in self.evtstore.proxies():
            if not p.isValid():
                continue
            keys.append(p.name())
        for br in self.activeBranches:
            try:
                if br not in keys:
                    raise KeyError("no such object [%s] in store" % br)
                o = self.evtstore[br]
                if hasattr(o, 'at'):
                    o = list(o)
                    for i,v in enumerate(o):
                        if hasattr(v, 'at') and not isinstance(v, str):
                            o[i] = list(v)
                self.msg.info('%s: %r', br, o)
                print ("%s: %r" % (br, o), file=self.fd)
            except Exception as err:
                self.msg.info(' --> err for [%s]: %s', br, err)
                pass
        return StatusCode.Success

    def finalize(self):
        if hasattr(self, 'fd') and self.fd:
            self.fd.flush()
            self.fd.close()
        return StatusCode.Success

    pass # MyAlg


# Create ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaRootComps.RootReadConfig import RootReadCfg

acc = MainServicesCfg(flags)
acc.merge( RootReadCfg(flags, tupleName="egamma") )

# Add Python algorithm
acc.addEventAlgo( MyAlg('py_alg', activeBranches=args.branches.split(',')) )

# Add DecisionSvc
acc.addService(CompFactory.DecisionSvc())

# Configure output stream
from AthenaRootComps.RootWriteConfig import NtupleOutputStreamCfg
acc.merge( NtupleOutputStreamCfg(flags,
                                 streamName="StreamD3PD",
                                 fileName="d3pd.root",
                                 tupleName="egamma",
                                 ItemList=args.outbranches.split(',')) )

# Run
if __name__ == "__main__":
    sys.exit(acc.run().isFailure())
