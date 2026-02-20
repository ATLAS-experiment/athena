# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddTrackSummaryAlgCfg(flags, name="AddTrackSummaryAlg", **kwargs):
    """
    Configure the AddTrackSummaryAlg to add TrackSummary to tracks read from file
    
    This is needed for track overlay where pileup tracks don't have TrackSummary
    objects (not persisted with Track EDM).
    """
    result = ComponentAccumulator()
    
    # Get the track summary tool
    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", 
                         result.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))
    
    # Create the algorithm
    result.addEventAlgo(CompFactory.Trk.AddTrackSummaryAlg(name, **kwargs))
    
    return result

