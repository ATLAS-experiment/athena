# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineStreamToServerCfg(flags, OnlineEventDisplaysSvc = None):
    acc = ComponentAccumulator()
    
    from JiveXML.ExternalONCRPCServerSvcConfig import ExternalONCRPCServerSvcCfg
    acc.merge(ExternalONCRPCServerSvcCfg(flags))

    streamToServerTool = CompFactory.JiveXML.StreamToServerTool(name='OnlineStreamToFileTool',
                                                                StreamName = ".Unknown",
                                                                OnlineEventDisplaysSvc = OnlineEventDisplaysSvc)
    acc.setPrivateTools(streamToServerTool)
                                                            
    return acc
