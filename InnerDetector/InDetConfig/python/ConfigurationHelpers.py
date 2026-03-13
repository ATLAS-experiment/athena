"""Disable everything but track reconstruction

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

def OnlyTrackingPreInclude(flags):
    """
    This will manually disable everything except for tracking.
    The purpose is to speed up overall execution by only running 
    tracking and nothing else
    """
    flags.Reco.EnableBTagging=False
    flags.Reco.EnableCombinedMuon=False
    flags.Reco.EnableEgamma=False
    flags.Reco.EnableJet=False
    flags.Reco.EnableTau=False
    flags.Reco.EnablePFlow=False
    flags.Reco.EnableTrigger=False
    flags.Reco.EnableTracking=True
