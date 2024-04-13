# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ByteStreamCfg(flags, partitionName = '', streamsWanted=[], isBeamSplashMode = False):

    acc = ComponentAccumulator()

    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(flags))

    bytestreamInput = acc.getService("ByteStreamInputSvc")
    bytestreamInput.Partition = partitionName
    bytestreamInput.GroupName = "EventDisplaysOnline"
    bytestreamInput.PublishName = "EventDisplays"
    bytestreamInput.Key = "dcm"
    bytestreamInput.KeyCount = 3
    bytestreamInput.Timeout = 600000
    bytestreamInput.UpdatePeriod = 200
    bytestreamInput.BufferSize = 10 # three times of keycount for beam splashes
    bytestreamInput.ISServer = '' # Disable histogramming
    bytestreamInput.StreamNames = streamsWanted
    #bytestreamInput.StreamType = "physics" #comment out for all streams, e.g. if you also want claibration streams
    bytestreamInput.StreamLogic = "Or"

    if isBeamSplashMode:
        bytestreamInput.KeyCount = 62 # equal or greater than the number of DCMs for beam splashes
        bytestreamInput.BufferSize = 186 # three times of keycount for beam splashes
        bytestreamInput.Timeout = 144000000 #(40 hrs) for beam splashes
        bytestreamInput.StreamType = "physics" #if trigger fails it will go to debug_HltError

    if partitionName != 'ATLAS':
        bytestreamInput.KeyValue = [ 'Test_emon_push' ]
        bytestreamInput.KeyCount = 1

    return acc
