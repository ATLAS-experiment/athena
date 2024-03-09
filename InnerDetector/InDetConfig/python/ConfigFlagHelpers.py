# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


def innerDetectorOnly(flags):
    """Only run inner detector tracking"""
    flags.Detector.EnableMuon = False
    flags.Detector.EnableCalo = True
    flags.Reco.EnableTracking = True
    flags.Reco.EnableCombinedMuon = False
    flags.Reco.EnableEgamma = False
    flags.Reco.EnablePFlow = False
    flags.Reco.EnableTau = False
    flags.Reco.EnableJet = False
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableTrigger = False
    flags.Reco.EnablePostProcessing = False
