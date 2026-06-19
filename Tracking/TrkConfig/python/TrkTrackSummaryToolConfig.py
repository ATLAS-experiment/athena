# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkTrackSummaryTool package
# In the current workflow, we need to add information
# to the Trk::Track(s) as we build them.
#
# Given the set of measurement and parametersin a Trk::Track
# We always add
# - The number of hits Pixel/SCT/TRT (default)
# We optionally can add
# - Holes  in the measurement set (default False)
# the fitted perigee parameters (default False)
# - Muon specific additional information
#
# Note that we try to avoid the Hole search if
# is not absolutely needed as is expensive.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def InDetTrackSummaryToolCfg(flags, name='InDetTrackSummaryTool', **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkTrackSummaryToolCfg(flags, name, **kwargs)

    acc = ComponentAccumulator()

    kwargs.setdefault("doHolesInDet", True)

    if 'InDetSummaryHelperTool' not in kwargs:
        if kwargs["doHolesInDet"]:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import (
                InDetTrackSummaryHelperToolCfg)
            InDetSummaryHelperTool = acc.popToolsAndMerge(
                InDetTrackSummaryHelperToolCfg(flags))
        else:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import (
                InDetSummaryHelperNoHoleSearchCfg)
            InDetSummaryHelperTool = acc.popToolsAndMerge(
                InDetSummaryHelperNoHoleSearchCfg(flags))
        kwargs.setdefault("InDetSummaryHelperTool", InDetSummaryHelperTool)

    acc.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return acc

def InDetTrackSummaryToolNoHoleSearchCfg(
        flags, name='InDetTrackSummaryToolNoHoleSearch', **kwargs):
    kwargs.setdefault('doHolesInDet', False)
    return InDetTrackSummaryToolCfg(flags, name, **kwargs)


def InDetTrigTrackSummaryToolCfg(
        flags,name="InDetTrigTrackSummaryTool",**kwargs):
    """
    instance with hole search
    """
    acc = ComponentAccumulator()

    kwargs.setdefault("doHolesInDet", True)

    if 'InDetSummaryHelperTool' not in kwargs:
        from InDetConfig.InDetTrackSummaryHelperToolConfig import (
            TrigTrackSummaryHelperToolCfg)
        kwargs.setdefault("InDetSummaryHelperTool", acc.popToolsAndMerge(
            TrigTrackSummaryHelperToolCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return acc

def InDetTrigFastTrackSummaryToolCfg(
        flags, name="InDetTrigFastTrackSummaryTool", **kwargs):
    """
    faster instance without hole search and TRT 
    """

    acc = ComponentAccumulator()

    from InDetConfig.InDetTrackSummaryHelperToolConfig import (
        TrigTrackSummaryHelperToolSiOnlyCfg)

    kwargs.setdefault("doHolesInDet", False)
        
    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetTrigTrackSummaryToolCfg(
            flags, name,
            # Prevents summary helper tool to be incorrectly set to something
            # else through kwargs
            InDetSummaryHelperTool = acc.popToolsAndMerge(
                TrigTrackSummaryHelperToolSiOnlyCfg(flags)),
            **kwargs)))
    return acc

def ITkTrackSummaryToolCfg(flags, name='ITkTrackSummaryTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("doHolesInDet", not flags.Tracking.doITkFastTracking)

    if 'InDetSummaryHelperTool' not in kwargs:
        if kwargs["doHolesInDet"]:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import (
                ITkTrackSummaryHelperToolCfg)
            ITkSummaryHelperTool = acc.popToolsAndMerge(
                ITkTrackSummaryHelperToolCfg(flags))
        else:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import (
                ITkSummaryHelperNoHoleSearchCfg)
            ITkSummaryHelperTool = acc.popToolsAndMerge(
                ITkSummaryHelperNoHoleSearchCfg(flags))
        kwargs.setdefault("InDetSummaryHelperTool", ITkSummaryHelperTool)

    acc.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return acc

def ITkTrackSummaryToolNoHoleSearchCfg(
        flags, name='ITkTrackSummaryToolNoHoleSearch', **kwargs):
    kwargs.setdefault('doHolesInDet', False)
    return ITkTrackSummaryToolCfg(flags, name, **kwargs)


def GSFTrackSummaryToolCfg(
        flags, name="GSFTrackSummaryTool", **kwargs):
    """ The Track Summary for the GSF refitted Tracks/TrackParticles"""

    acc = ComponentAccumulator()

    if "InDetSummaryHelperTool" not in kwargs:
        from InDetConfig.InDetTrackSummaryHelperToolConfig import (
            InDetTrackSummaryHelperToolCfg)
        kwargs["InDetSummaryHelperTool"] = acc.popToolsAndMerge(
            InDetTrackSummaryHelperToolCfg(
                flags,
                name="GSFBuildTrackSummaryHelperTool",
                HoleSearch=None))

    kwargs.setdefault("doHolesInDet", False)

    # Particle creator needs a public one
    acc.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return acc

@AccumulatorCache
def MuonTrackSummaryToolCfg(flags, name="MuonTrackSummaryTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonRecToolsConfig import MuonTrackSummaryHelperToolCfg
    kwargs.setdefault("MuonSummaryHelperTool", result.popToolsAndMerge(
        MuonTrackSummaryHelperToolCfg(flags)))
    kwargs.setdefault("AddDetailedMuonSummary", True)
    result.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return result

@AccumulatorCache
def MuonCombinedTrackSummaryToolCfg(
        flags, name="CombinedMuonTrackSummary", **kwargs):

    result = ComponentAccumulator()

    if "InDetSummaryHelperTool" not in kwargs:
        from InDetConfig.InDetTrackSummaryHelperToolConfig import (
            CombinedMuonIDSummaryHelperToolCfg)
        kwargs.setdefault("InDetSummaryHelperTool", result.popToolsAndMerge(
            CombinedMuonIDSummaryHelperToolCfg(flags)))

    if "MuonSummaryHelperTool" not in kwargs:
        from MuonConfig.MuonRecToolsConfig import MuonTrackSummaryHelperToolCfg
        kwargs.setdefault("MuonSummaryHelperTool", result.popToolsAndMerge(
            MuonTrackSummaryHelperToolCfg(flags)))

    kwargs.setdefault("doHolesInDet", True)
    kwargs.setdefault("doHolesMuon", False)
    kwargs.setdefault("AddDetailedMuonSummary", True)
    kwargs.setdefault("PixelExists", True)

    result.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))
    return result

@AccumulatorCache
def CombinedSummaryToolCfg(flags, name="CombinedTrackSummaryTool",**kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("doHolesInDet", flags.Detector.GeometryITk or flags.Detector.GeometryID)
    if flags.Detector.GeometryITk:
        if kwargs["doHolesInDet"]:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import ITkTrackSummaryHelperToolCfg
            ITkSummaryHelperTool = result.popToolsAndMerge(ITkTrackSummaryHelperToolCfg(flags))
        else:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import ITkSummaryHelperNoHoleSearchCfg
            ITkSummaryHelperTool = result.popToolsAndMerge(ITkSummaryHelperNoHoleSearchCfg(flags))
        kwargs.setdefault("InDetSummaryHelperTool", ITkSummaryHelperTool)
    elif flags.Detector.GeometryID:
        if kwargs["doHolesInDet"]:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import InDetTrackSummaryHelperToolCfg
            InDetSummaryHelperTool = result.popToolsAndMerge(InDetTrackSummaryHelperToolCfg(flags))
        else:
            from InDetConfig.InDetTrackSummaryHelperToolConfig import InDetSummaryHelperNoHoleSearchCfg
            InDetSummaryHelperTool = result.popToolsAndMerge(InDetSummaryHelperNoHoleSearchCfg(flags))
        kwargs.setdefault("InDetSummaryHelperTool", InDetSummaryHelperTool)
    #### muon configuration
    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonRecToolsConfig import MuonTrackSummaryHelperToolCfg
        kwargs.setdefault("MuonSummaryHelperTool", result.popToolsAndMerge(MuonTrackSummaryHelperToolCfg(flags)))

    kwargs.setdefault("AddDetailedMuonSummary", flags.Detector.GeometryMuon)
    
    result.setPrivateTools(CompFactory.Trk.TrackSummaryTool(name, **kwargs))

    return result