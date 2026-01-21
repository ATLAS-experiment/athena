#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Detector.GeometryITkStrip = True

    flags.ITk.Geometry.AllLocal = False

    # for debugging
    #from AthenaCommon.Constants import DEBUG
    #flags.Exec.OutputLevel=DEBUG
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For POOL file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # For ByteStream file writing
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    writingAcc = ByteStreamWriteCfg(flags, [ "SCT_RDO_Container#ITkStripRDOs" ] )
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamType = "EventStorage"
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamName = "StreamBSFileOutput"
    acc.merge(writingAcc)

    from ITkStripsByteStreamCnv.ITkStripByteStreamCnvConfig import ITkStripRawContByteStreamToolProviderToolCfg
    acc.merge(ITkStripRawContByteStreamToolProviderToolCfg(flags))        

    # For EventInfo necessary for ByteStream file writing
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    acc.merge(EventInfoCnvAlgCfg(flags,
                                 inputKey="McEventInfo",
                                 outputKey="EventInfo"))
    
    acc.run(maxEvents=1)
