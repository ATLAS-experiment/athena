#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

def OnlineStreamToFileCfg(flags, OnlineEventDisplaysSvc = None):
    acc = ComponentAccumulator()
    streamToFileTool = CompFactory.JiveXML.StreamToFileTool(name='OnlineStreamToFileTool',
                                                            OnlineEventDisplaysSvc = OnlineEventDisplaysSvc,
                                                            IsOnline = True,
                                                            OutputLevel = DEBUG)
    acc.setPrivateTools(streamToFileTool)
    return acc
