# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Configuration of ZWindowRoISeedTool package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Setup per-event calculation of a region-of-interest (RoI) along z-axis
def LeadTracksRoISeedToolCfg(flags, **kwargs) :
    acc = ComponentAccumulator()

    # Set up needed tools
    if "TrackToVertexTool" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        kwargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(
            InDetTrackToVertexCfg(flags)))

    # Set input/output
    kwargs.setdefault("InputTracksCollection", "ExtendedTracks")
    kwargs.setdefault("BeamSpotKey", "BeamSpotData")

    # Set other input properties
    kwargs.setdefault("TrackZ0Window", flags.Tracking.ActiveConfig.z0WindowRoI)

    acc.setPrivateTools(CompFactory.InDet.LeadTracksRoISeedTool(
        "InDetLeadTracksRoISeedTool"+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc

def RandomRoISeedToolCfg(flags, **kwargs) :
    acc = ComponentAccumulator()

    # Set input/output                                                                                                                                                      
    kwargs.setdefault("BeamSpotKey", "BeamSpotData")

    # Set other input properties                                                                                                                                            
    kwargs.setdefault("TrackZ0Window", flags.Tracking.ActiveConfig.z0WindowRoI)

    acc.setPrivateTools(CompFactory.InDet.RandomRoISeedTool(
        "InDetRandomRoISeedTool"+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc

def FileRoISeedToolCfg(flags, **kwargs) :
    acc = ComponentAccumulator()

    # Set input/output                                                                                                                                                      
    kwargs.setdefault("InputFileName", flags.Tracking.ActiveConfig.inputLowPtRoIfile)

    # Set other input properties                                                                                                                                            
    kwargs.setdefault("TrackZ0Window", flags.Tracking.ActiveConfig.z0WindowRoI)

    acc.setPrivateTools(CompFactory.InDet.FlatRoISeedTool(
        "InDetFlatRoISeedTool"+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc

def TruthHSRoISeedToolCfg(flags, **kwargs) :
    acc = ComponentAccumulator()

    # Set input/output                                                                                                                                                      
    kwargs.setdefault("InputTruthEventsCollection", "TruthEvents")

    # Set other input properties                                                                                                                                            
    kwargs.setdefault("TrackZ0Window", flags.Tracking.ActiveConfig.z0WindowRoI)

    acc.setPrivateTools(CompFactory.InDet.TruthHSRoISeedTool(
        "InDetTruthHSRoISeedTool"+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc
