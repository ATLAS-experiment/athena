# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetectorBuilderToolCfg(flags, name="MuonDetectorBuilderTool", **kwargs):
    result = ComponentAccumulator()
    theTool = CompFactory.ActsTrk.MuonDetectorBuilderTool(name, **kwargs)
    result.addPublicTool(theTool, primary = True)
    return result