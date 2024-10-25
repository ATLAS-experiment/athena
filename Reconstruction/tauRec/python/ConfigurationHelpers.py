"""Enable dependencies of tau reconstruction

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

def StandaloneTauRecoFlags(flags):

    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = True
    flags.Reco.EnablePFlow = True
    flags.Reco.EnableTau = True
    flags.Reco.EnableJet = True
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableCaloRinger = False
    flags.Reco.PostProcessing.GeantTruthThinning = False
    flags.Reco.PostProcessing.TRTAloneThinning = False
