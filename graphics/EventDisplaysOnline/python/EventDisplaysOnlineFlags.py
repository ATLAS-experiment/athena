# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createOnlineEventDisplayFlags():
    flags=AthConfigFlags()
    flags.addFlag('OnlineEventDisplays.MakeVP1File', False)
    flags.addFlag('OnlineEventDisplays.Ready4PhysicsAtStart', False)
    flags.addFlag('OnlineEventDisplays.BeamSplashMode', False)
    flags.addFlag('OnlineEventDisplays.HorizontalMuonsMode', False)
    flags.addFlag('OnlineEventDisplays.HIMode', False)
    flags.addFlag('OnlineEventDisplays.HIPMode', False)
    flags.addFlag('OnlineEventDisplays.CosmicMode',False)
    flags.addFlag('OnlineEventDisplays.PartitionName', 'ATLAS')
    flags.addFlag('OnlineEventDisplays.OfflineTest', False, help='Run the online event displays reco outside of point 1.')
    flags.addFlag('OnlineEventDisplays.TriggerStreams', ['MinBias','express','ZeroBias','CosmicCalo','IDCosmic','CosmicMuons','Background','Standby','L1Calo','Main'])
    flags.addFlag('OnlineEventDisplays.OutputDirectory', '/atlas/EventDisplayEvents/')
    flags.addFlag('OnlineEventDisplays.MaxEvents', 100, help= 'Number of events to keep per stream in /atlas/EventDisplays/streamName')
    flags.addFlag('OnlineEventDisplays.PublicStreams',[''], help = 'Trigger streams that can be seen by the general public')
    flags.addFlag('OnlineEventDisplays.ProjectTag', "")
    return flags
