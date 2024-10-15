# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetectorBuilderToolCfg(flags, name="MuonDetectorBuilderTool",
                               dumpDetector = False,
                               dumpPassive = False,
                               dumpDetectorVolumes = False,
                               **kwargs):
    result = ComponentAccumulator()
    kwargs['dumpDetector'] = dumpDetector
    kwargs['dumpPassive'] = dumpPassive
    kwargs['dumpDetectorVolumes'] = dumpDetectorVolumes
    theTool = CompFactory.ActsTrk.MuonDetectorBuilderTool(name, **kwargs)
    result.addPublicTool(theTool, primary = True)
    return result
