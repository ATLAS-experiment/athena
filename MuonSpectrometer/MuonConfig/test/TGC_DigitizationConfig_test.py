#!/usr/bin/env python
"""Run tests on TGC_DigitizationConfig.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultTestFiles
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from MuonConfig.TGC_DigitizationConfig import TGC_DigitizerDigitToRDOCfg

# Set up logging
log.setLevel(DEBUG)
# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_MC
flags.Output.RDOFileName = "myRDO.pool.root"
flags.lock()
# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))
acc.merge(TGC_DigitizerDigitToRDOCfg(flags))
# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
flags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())
