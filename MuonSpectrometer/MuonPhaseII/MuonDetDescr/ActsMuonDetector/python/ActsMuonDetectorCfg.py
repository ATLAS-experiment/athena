# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetectorBuilderToolCfg(flags, name="MuonDetectorBuilderTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('dumpDetector', False)
    kwargs.setdefault('dumpPassive', False)
    kwargs.setdefault('dumpDetectorVolumes', False)
    theTool = CompFactory.ActsTrk.MuonDetectorBuilderTool(name, **kwargs)
    result.addPublicTool(theTool, primary = True)
    return result

def MsTrackingVolumeBuilderCfg(flags, name = "MSTrackingVolumeBuilder", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_tool = CompFactory.ActsTrk.MSTrackingVolumeBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonBlueprintNodeBuilderCfg(flags, name = "MuonBlueprintNodeBuilder", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_tool = CompFactory.ActsTrk.MuonBlueprintNodeBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

