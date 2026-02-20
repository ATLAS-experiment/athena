#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
## Test to read ROOT objects via ITHistSvc

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Exec.MaxEvents = 1
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)

from AthenaPython.tests.PyTHistTestsLib import PyHistReader
cfg.addEventAlgo(PyHistReader())

# define histsvc {in/out}put streams
cfg.addService(CompFactory.THistSvc(Input = ["read1 DATAFILE='tuple1.root' OPT='READ'",
                                             "read2 DATAFILE='tuple2.root' OPT='READ'"],
                                    PrintAll = True))

import sys
sys.exit(cfg.run().isFailure())
