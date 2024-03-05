#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(flags)

    # enable tau reconstruction
    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = True
    flags.Reco.EnablePFlow = True
    flags.Reco.EnableTau = True
    flags.Reco.EnableJet = True
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableCaloRinger = False
    flags.Reco.PostProcessing.GeantTruthThinning = False
    flags.Reco.PostProcessing.TRTAloneThinning = False

    flags.fillFromArgs()
    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    #with open("config.pkl", "wb") as file:
    #  acc.store(file)

    acc.run(100)
