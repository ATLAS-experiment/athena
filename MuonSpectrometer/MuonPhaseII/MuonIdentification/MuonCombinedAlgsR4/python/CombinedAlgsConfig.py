# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonCreatorAlgCfg(flags, name="MuonCreatorAlgR4", **kwargs):
    result = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("SelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags)))
    from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.MuonCreatorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result