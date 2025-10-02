#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Input.Files = []
flags.Input.RunNumbers = [1]  # avoid input file peeking
flags.Input.TypedCollections = []
flags.Exec.MaxEvents = 5
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
cfg = MainEvgenServicesCfg(flags)

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
cfg.merge( EventInfoCnvAlgCfg(flags, disableBeamSpot = True) )

# Random number service
cfg.addService( CompFactory.AtRndmGenSvc(
    Seeds = ["PYTHIA 4789899 989240512", "PYTHIA_INIT 820021 2347532",
             "JIMMY 390020611 821000366", "JIMMY_INIT 820021 2347532",
             "HERWIG 390020611 821000366", "HERWIG_INIT 820021 2347532"]) )

# Create some 4-vectors
cfg.addEventAlgo( CompFactory.AthExThinning.CreateData(
    "CreateData",
    ParticlesOutput = "Particles",
    NbrOfParticles  = 10) )

# Pool Persistency
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
cfg.merge( OutputStreamCfg(flags,
                           streamName = "ToThin",
                           disableEventTag = True,
                           ItemList = ["EventInfo#*",
                                       "AthExParticles#*",
                                       "AthExDecay#*",
                                       "AthExElephantino#*"]) )

import sys
sys.exit( cfg.run().isFailure() )
