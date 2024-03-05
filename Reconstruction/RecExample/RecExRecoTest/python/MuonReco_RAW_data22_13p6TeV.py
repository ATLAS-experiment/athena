#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(flags)
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
    acc.merge(MuonReconstructionCfg(flags))

    acc.run(100)
