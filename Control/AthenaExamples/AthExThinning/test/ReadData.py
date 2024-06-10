#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Input.ProcessingTags = []
flags.addFlag("Input.Mode", "Thinned")  # or "NonThinned"
flags.fillFromArgs()

if flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']:
   if flags.Input.Mode == "Thinned":
      flags.Input.Files = ["myUSR_0.pool.root"]
   elif flags.Input.Mode == "NonThinned":
      flags.Input.Files = ["myUSR_1.pool.root"]

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

# Pool reading
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge( PoolReadCfg(flags) )

if flags.Input.Mode == "NonThinned":
   from AthExThinning.Lib import PyReadNonThinnedData as Reader
elif flags.Input.Mode == "Thinned":
   Reader = CompFactory.AthExThinning.ReadThinnedData

cfg.addEventAlgo( Reader(
   f"Read{flags.Input.Mode}Data",
   Particles   = "Particles",
   Decay       = "TwoBodyDecay",
   Elephantino = "PinkElephantino") )

# Pool Persistency
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
cfg.merge( OutputStreamCfg(flags,
                           streamName = f"Reaccessed{flags.Input.Mode}",
                           disableEventTag = True,
                           ItemList = ["EventInfo#*",
                                       "AthExParticles#*",
                                       "AthExDecay#*",
                                       "AthExElephantino#*"]) )

import sys
sys.exit( cfg.run().isFailure() )
