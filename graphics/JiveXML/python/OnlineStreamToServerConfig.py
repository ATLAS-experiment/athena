# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineStreamToServerCfg(flags, name='OnlineStreamToServerTool', **kwargs):
    acc = ComponentAccumulator()

    if "OnlineEventDisplaysSvc" not in kwargs:
        from EventDisplaysOnline.OnlineEventDisplaysSvcConfig import OnlineEventDisplaysSvcCfg
        acc.merge(OnlineEventDisplaysSvcCfg(flags))
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))

    serverService = CompFactory.JiveXML.ExternalONCRPCServerSvc(name="ExternalONCRPCServerSvc", Hostname = "pc-tdq-mon-29")
    acc.addService(serverService)
    kwargs.setdefault("ServerService",serverService)
    kwargs.setdefault("StreamName",".Unknown")
    the_tool = CompFactory.JiveXML.StreamToServerTool(name,**kwargs)
    acc.setPrivateTools(the_tool)
    return acc
