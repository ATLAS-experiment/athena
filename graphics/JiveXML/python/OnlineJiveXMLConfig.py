# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def OnlineJiveXMLCfg(flags, StreamToFileTool = None, StreamToServerTool = None, OnlineMode = True):

    acc = ComponentAccumulator()
    
    from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
    acc.merge(AlgoJiveXMLCfg(flags,
                             StreamToFileTool = StreamToFileTool,
                             StreamToServerTool = StreamToServerTool,
                             OnlineMode = OnlineMode,
                             OutputLevel = DEBUG))

    return acc
