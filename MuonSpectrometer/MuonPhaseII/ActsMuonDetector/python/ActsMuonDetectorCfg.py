# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetectorBuilderToolCfg(flags, name="MuonDetectorBuilderTool",
                               DumpVisualization = False,
                               **kwargs):
    result = ComponentAccumulator()
    kwargs['DumpVisualization'] = DumpVisualization
    theTool = CompFactory.ActsTrk.MuonDetectorBuilderTool(name, **kwargs)
    result.addPublicTool(theTool, primary = True)
    return result
