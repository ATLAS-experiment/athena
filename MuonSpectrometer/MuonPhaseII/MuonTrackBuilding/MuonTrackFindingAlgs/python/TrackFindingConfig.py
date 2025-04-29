# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SegmentSelectorCfg(flags, name="SegmentSelectionTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonR4.SegmentSelectionTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MSTrackFinderAlgCfg(flags, name="MSTrackFinderAlg", **kwargs):
    result = ComponentAccumulator()
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
    segmentKeys = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        segmentKeys+=["MuonSegmentsFromR4"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        segmentKeys+=[]## Needs to be filled once the MM segments are ready

    kwargs.setdefault("SegmentContainer", segmentKeys)
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    the_alg = CompFactory.MuonR4.MSTrackFindingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result