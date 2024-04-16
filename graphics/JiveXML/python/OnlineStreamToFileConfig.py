#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineStreamToFileCfg(flags,name='OnlineStreamToFileTool', **kwargs):
    acc = ComponentAccumulator()

    if "OnlineEventDisplaysSvc" not in kwargs:
        from EventDisplaysOnline.OnlineEventDisplaysSvcConfig import OnlineEventDisplaysSvcCfg
        acc.merge(OnlineEventDisplaysSvcCfg(flags))
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))

    kwargs.setdefault("IsOnline", True)

    streamToFileTool = CompFactory.JiveXML.StreamToFileTool(name, **kwargs)
    acc.setPrivateTools(streamToFileTool)
    return acc
