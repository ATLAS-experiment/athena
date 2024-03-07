#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultTestFiles

flags = initConfigFlags()
flags.Concurrency.NumThreads = 10
flags.Concurrency.NumConcurrentEvents = 2
flags.Exec.MaxEvents = 1000
flags.Input.Files = defaultTestFiles.RAW_RUN3
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
acc.merge(ByteStreamReadCfg(flags))

acc.addEventAlgo(CompFactory.ROBDataProviderMTTest())

import sys
sys.exit(acc.run().isFailure())
