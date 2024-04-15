#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

def OnlineStreamToFileCfg(flags,name='OnlineStreamToFileTool', **kwargs):
    acc = ComponentAccumulator()

    if "OnlineEventDisplaysSvc" not in kwargs:
        from EventDisplaysOnline.EventDisplaysOnlineConfig import OnlineEventDisplaysSvcCfg
        acc.merge(OnlineEventDisplaysSvcCfg(flags))
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))

    kwargs.setdefault("IsOnline", True)

    streamToFileTool = CompFactory.JiveXML.StreamToFileTool(name, OutputLevel = DEBUG, **kwargs)
    acc.setPrivateTools(streamToFileTool)
    return acc
