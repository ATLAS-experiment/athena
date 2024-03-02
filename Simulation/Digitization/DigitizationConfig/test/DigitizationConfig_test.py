#!/usr/bin/env python
"""Test various ComponentAccumulator Digitization configuration modules

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from DigitizationConfig.DigitizationSteering import DigitizationMainCfg, DigitizationMessageSvcCfg

# Set up logging
log.setLevel(DEBUG)

# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.Output.RDOFileName = "myRDO.pool.root"
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-25-02"
flags.GeoModel.Align.Dynamic = False
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents=1
flags.Beam.NumberOfCollisions = 0.

flags.lock()

# Construct our accumulator to run
acc = DigitizationMainCfg(flags)
acc.merge(DigitizationMessageSvcCfg(flags))

# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
flags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())
