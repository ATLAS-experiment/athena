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
