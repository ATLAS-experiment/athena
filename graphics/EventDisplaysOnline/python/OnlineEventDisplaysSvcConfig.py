# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def OnlineEventDisplaysSvcCfg(flags, maxEvents = 100, outputDirectory = '', sendToPublicStream = False, publicStreams = [], streamsWanted = [], isBeamSplashMode = False):

    acc = ComponentAccumulator()

    onlineEventDisplaysSvc = CompFactory.OnlineEventDisplaysSvc(
        name = "OnlineEventDisplaysSvc",
        MaxEvents = maxEvents,                   # Number of events to keep per stream
        OutputDirectory = outputDirectory,       # Base directory for streams
        SendToPublicStream = sendToPublicStream, # Allowed to be made public
        PublicStreams = publicStreams,           # These streams go into public stream when Ready4Physics
        StreamsWanted = streamsWanted,
        BeamSplash = isBeamSplashMode
    )
    acc.addService(onlineEventDisplaysSvc, create=True)

    return acc
