# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTruthSelectionToolTrackOverlayCfg(flags, name="AthTruthSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("requireCharged", flags.PhysVal.IDPVM.requireCharged)
    acc.setPrivateTools(CompFactory.AthTruthSelectionTool(name, **kwargs))
    return acc

def TrackOverlayDecisionAlgCfg(flags, name="TrackOverlayDecisionAlg",  **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(InDetTruthSelectionToolTrackOverlayCfg(flags)))
    kwargs.setdefault("MLThreshold", flags.TrackOverlay.MLThreshold)
    acc.addEventAlgo(CompFactory.TrackOverlayDecisionAlg.TrackOverlayDecisionAlg(name, **kwargs))
    return acc

def InvertedTrackOverlayDecisionAlgCfg(flags, name="InvertedTrackOverlayDecisionAlg", **kwargs):
    kwargs.setdefault("InvertFilter", True)
    return TrackOverlayDecisionAlgCfg(flags, name,**kwargs)

