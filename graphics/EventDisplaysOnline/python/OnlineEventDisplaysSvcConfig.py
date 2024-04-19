# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineEventDisplaysSvcCfg(flags, name = "OnlineEventDisplaysSvc", **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("MaxEvents", flags.OnlineEventDisplays.MaxEvents)
    kwargs.setdefault("OutputDirectory", flags.OnlineEventDisplays.OutputDirectory)
    kwargs.setdefault("ProjectTag", flags.OnlineEventDisplays.ProjectTag)
    kwargs.setdefault("StreamsWanted", flags.OnlineEventDisplays.TriggerStreams)
    kwargs.setdefault("BeamSplash", flags.OnlineEventDisplays.BeamSplashMode)
    kwargs.setdefault("CheckPair", False)
    onlineEventDisplaysSvc = CompFactory.OnlineEventDisplaysSvc(name, **kwargs)

    acc.addService(onlineEventDisplaysSvc, create=True)

    return acc
