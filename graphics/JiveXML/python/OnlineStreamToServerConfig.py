# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineStreamToServerCfg(flags, name='OnlineStreamToFileTool', **kwargs):
    acc = ComponentAccumulator()

    if "ExternalONCRPCServerSvc" not in kwargs:
        from JiveXML.ExternalONCRPCServerSvcConfig import ExternalONCRPCServerSvcCfg
        acc.merge(ExternalONCRPCServerSvcCfg(flags))
        kwargs.setdefault("ExternalONCRPCServerSvc", acc.getService("ExternalONCRPCServerSvc"))

    if "OnlineEventDisplaysSvc" not in kwargs:
        from EventDisplaysOnline.OnlineEventDisplaysSvcConfig import OnlineEventDisplaysSvcCfg
        acc.merge(OnlineEventDisplaysSvcCfg(flags))
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))

    kwargs.setdefault("StreamName", ".Unknown")

    streamToServerTool = CompFactory.JiveXML.StreamToServerTool(name, **kwargs)

    acc.setPrivateTools(streamToServerTool)

    return acc
