#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Script to print HLT truncation info.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.fillFromArgs()
flags.lock()

# Define the decoding/analysis sequence
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
cfg.merge(ByteStreamReadCfg(flags))

from TriggerJobOpts.TriggerRecoConfig import Run3TriggerBSUnpackingCfg
cfg.merge(Run3TriggerBSUnpackingCfg(flags))

cfg.getEventAlgo("TrigDeserialiser").ExtraOutputs.add(
   ('xAOD::TrigCompositeContainer', 'StoreGateSvc+TruncationDebugInfo') )

cfg.addEventAlgo(CompFactory.TruncationAnalysisAlg("TruncationAnalysis"),
                 sequenceName="HLTDecodingSeq")

import sys
sys.exit(cfg.run().isFailure())
