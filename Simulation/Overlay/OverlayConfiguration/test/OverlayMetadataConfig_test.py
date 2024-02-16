#!/usr/bin/env python
"""Run tests for overlay metadata

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg

from DigitizationConfig.DigitizationParametersConfig import writeDigitizationParameters
from OverlayConfiguration.OverlayMetadata import overlayMetadataCheck
from OverlayConfiguration.OverlayTestHelpers import overlayTestFlags, postprocessAndLockFlags, printAndRun, CommonTestArgumentParser

# Argument parsing
parser = CommonTestArgumentParser("OverlayMetadataConfig_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
overlayTestFlags(flags, args)
overlayMetadataCheck(flags)
postprocessAndLockFlags(flags, args)
flags.initAll()
flags.dump()
# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))
acc.merge(writeDigitizationParameters(flags))

# Add event info overlay for minimal output
acc.merge(EventInfoOverlayCfg(flags))

# Print and run
sys.exit(printAndRun(acc, flags, args))
