# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def egammaOnlyFromRaw(flags):
    """egammaOnlyFromRaw flags for Reco_tf with CA"""
    flags.Detector.GeometryMuon = False
    flags.Detector.EnableAFP = False
    flags.Detector.EnableLucid = False
    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = False
    flags.Reco.EnablePFlow = False
    flags.Reco.EnableTau = False
    flags.Reco.EnableJet = False
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableCaloRinger = False
    flags.Egamma.doTrackThinning = False
    flags.Reco.PostProcessing.GeantTruthThinning = False
    flags.Reco.PostProcessing.TRTAloneThinning = False
