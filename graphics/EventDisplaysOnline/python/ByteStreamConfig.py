# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ByteStreamCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(flags))

    bytestreamInput = acc.getService("ByteStreamInputSvc")
    bytestreamInput.Partition = flags.OnlineEventDisplays.PartitionName
    bytestreamInput.GroupName = "EventDisplaysOnline"
    bytestreamInput.PublishName = "EventDisplays"
    bytestreamInput.Key = "dcm"
    bytestreamInput.KeyCount = 1
    bytestreamInput.Timeout = 600000
    bytestreamInput.UpdatePeriod = 200
    bytestreamInput.BufferSize = 10
    bytestreamInput.ISServer = '' # Disable histogramming
    bytestreamInput.StreamNames = flags.OnlineEventDisplays.TriggerStreams
    #bytestreamInput.StreamType = "physics" #comment out for all streams, e.g. if you also want claibration streams
    bytestreamInput.StreamLogic = "Or"

    if flags.OnlineEventDisplays.BeamSplashMode:
        bytestreamInput.KeyCount = 64 # equal or greater than the number of DCMs for beam splashes
        bytestreamInput.BufferSize = 192 # at least three times of keycount for beam splashes
        bytestreamInput.Timeout = 144000000 #(40 hrs)
        bytestreamInput.StreamType = "physics" #if trigger fails it will go to debug_HltError

    if flags.OnlineEventDisplays.PartitionName != 'ATLAS':
        bytestreamInput.KeyValue = [ 'Test_emon_push' ]
        bytestreamInput.KeyCount = 1

    return acc
