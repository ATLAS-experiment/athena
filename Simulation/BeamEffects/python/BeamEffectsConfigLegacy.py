# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Configurations for classes in BeamEffects package
"""

from AthenaCommon import CfgMgr

def getBeamSpotFixerAlg(name="BeamSpotFixerAlg", **kwargs):
    kwargs.setdefault('InputKey', 'Input_EventInfo')

    from Digitization.DigitizationFlags import digitizationFlags
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault('OutputKey', overlayFlags.bkgPrefix() + 'EventInfo')
    else:
        kwargs.setdefault('OutputKey', 'EventInfo')

    return CfgMgr.Simulation__BeamSpotFixerAlg(name, **kwargs)

def getBeamSpotReweightingAlg(name="BeamSpotReweightingAlg", **kwargs):
    from Digitization.DigitizationFlags import digitizationFlags
    kwargs.setdefault('Input_beam_sigma_z', digitizationFlags.OldBeamSpotZSize())

    return CfgMgr.Simulation__BeamSpotReweightingAlg(name, **kwargs)
