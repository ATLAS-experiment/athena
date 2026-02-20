#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
## Test write ROOT objects via ITHistSvc

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Exec.MaxEvents = 20
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)

from AthenaPython.tests.PyTHistTestsLib import PyHistWriter
cfg.addEventAlgo(PyHistWriter())

# define histsvc {in/out}put streams
cfg.addService(CompFactory.THistSvc(Output = ["upd DATAFILE='tuple1.root' OPT='UPDATE'",
                                              "rec DATAFILE='tuple2.root' OPT='RECREATE'"],
                                    PrintAll = True))

import sys
sys.exit(cfg.run().isFailure())
