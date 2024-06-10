#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
flags = initConfigFlags()
flags.addFlag("Exec.AlgMode", "cpp")
flags.Common.MsgSuppression = False
flags.Input.Files = ["myToThin.pool.root"]
flags.Input.ProcessingTags = []
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

# Pool reading
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge (PoolReadCfg (flags))

if flags.Exec.AlgMode == 'cpp':
   WriteThinnedData = CompFactory.AthExThinning.WriteThinnedData
else:
   from AthExThinning.Lib import PyWriteThinnedData as WriteThinnedData

cfg.addEventAlgo( WriteThinnedData(
   "WriteThinnedData",
   Particles   = "Particles",
   Decay       = "TwoBodyDecay",
   Elephantino = "PinkElephantino",
   Filter      = [ True,  False,  False, False, True,
                   False, False,  True,  True,  False ]) )

# Pool Persistency
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
itemList = [
   "AthExParticles#Particles_test1",
   "AthExParticles#Particles_test2",
   "AthExParticles#Particles_test3",
   "AthExDecay#TwoBodyDecay_test1",
   "AthExDecay#TwoBodyDecay_test2",
   "AthExDecay#TwoBodyDecay_test3",
   "AthExElephantino#PinkElephantino_test1",
   "AthExElephantino#PinkElephantino_test2",
   "AthExElephantino#PinkElephantino_test3",
]

cfg.merge( OutputStreamCfg(flags,
                           streamName = "USR_0",  # thinned stream
                           disableEventTag = True,
                           ItemList = itemList) )

cfg.merge( OutputStreamCfg(flags,
                           streamName = "USR_1",  # unthinned stream
                           disableEventTag = True,
                           ItemList = itemList) )

import sys
sys.exit( cfg.run().isFailure() )
