#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def EventDisplaysOnlineCfg(flags, **kwargs):
    from EventDisplaysOnline.EventDisplaysOnlineHelpers import GetBFields, WaitForPartition, GetUniqueJobID, GetRunNumber, Ready4Physics
    
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'EventDisplaysOnlineCfg' )

    if not flags.OnlineEventDisplays.OfflineTest:
        from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
        autoConfigOnlineRecoFlags(flags, flags.OnlineEventDisplays.PartitionName)
        flags.OnlineEventDisplays.Ready4PhysicsAtStart=Ready4Physics()

    # An explicit list for nominal data taking to exclude some high rate streams
    # Empty list to read all
    flags.OnlineEventDisplays.TriggerStreams = ['MinBias','express','ZeroBias','CosmicCalo','IDCosmic','CosmicMuons','Background','Standby','L1Calo','Main']
    if flags.OnlineEventDisplays.BeamSplashMode:
        flags.OnlineEventDisplays.TriggerStreams = ['MinBias'] #if trigger fails it will go to debug_HltError

    # If testing at p1, create dir /tmp/your_user_name and write out to /tmp/your_user_name to see output
    flags.OnlineEventDisplays.OutputDirectory = "/atlas/EventDisplayEvents/"

    if flags.OnlineEventDisplays.OfflineTest:
        flags.OnlineEventDisplays.OutputDirectory = "."

    ##----------------------------------------------------------------------##
    ## When the ATLAS partition is not running you can use two test         ##
    ## partitions that serve events from a raw data file.                   ##
    ## To see which files will be ran over on the test partitions, at       ##
    ## point 1, see the uncommented lines in:                               ##
    ## /det/dqm/GlobalMonitoring/GMTestPartition_oks/tdaq-12-00-00/         ##
    ## without_gatherer/GMTestPartitionT9.data.xml                          ##
    ## and in:                                                              ##
    ## /det/dqm/GlobalMonitoring/GMTextPartition_oks/tdaq-12-00-00/         ##
    ## without_gatherer/GMTestPartition.data.xml                            ##
    ##----------------------------------------------------------------------##
    flags.OnlineEventDisplays.PartitionName='ATLAS' # 'ATLAS', 'GMTestPartition' or 'GMTestPartitionT9'

    if flags.OnlineEventDisplays.HIMode:
        flags.OnlineEventDisplays.MaxEvents=200
        flags.OnlineEventDisplays.ProjectTag='data25_hi'
        flags.OnlineEventDisplays.PublicStreams=['HardProbes']
    if flags.OnlineEventDisplays.HIPMode:
        flags.OnlineEventDisplays.MaxEvents=200
        flags.OnlineEventDisplays.ProjectTag='data25_hip'
        flags.OnlineEventDisplays.PublicStreams=['Main']
    if flags.OnlineEventDisplays.CosmicMode:
        flags.OnlineEventDisplays.MaxEvents=200
        flags.OnlineEventDisplays.ProjectTag='data25_cos'
        flags.OnlineEventDisplays.PublicStreams=['']
    if flags.OnlineEventDisplays.BeamSplashMode:
        flags.OnlineEventDisplays.MaxEvents=-1 # keep all the events
        flags.OnlineEventDisplays.ProjectTag='data25_comm'
        flags.OnlineEventDisplays.PublicStreams=['']
    else:
        flags.OnlineEventDisplays.MaxEvents=50
        flags.OnlineEventDisplays.ProjectTag='data25_13p6TeV'
        flags.OnlineEventDisplays.PublicStreams=['Main']

    # Pause this thread until the partition is up
    if not flags.OnlineEventDisplays.OfflineTest:
        WaitForPartition(flags.OnlineEventDisplays.PartitionName)

    if not flags.OnlineEventDisplays.OfflineTest:
        import os
        IPC_timeout = int(os.environ['TDAQ_IPC_TIMEOUT'])
        print(" IPC_timeout Envrionment Variable = %d" %IPC_timeout)

    # Conditions tag
    if flags.OnlineEventDisplays.OfflineTest:
        flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2025-02'
    else:
        flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2025-01' # Online conditions tag

    # Geometry tag
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'

    flags.Trigger.triggerConfig='DB'

    jobId = GetUniqueJobID()

    # Test wth a small amount of events and write out to e.g. a tmp dir
    if flags.OnlineEventDisplays.PartitionName != 'ATLAS' or flags.OnlineEventDisplays.OfflineTest:
        flags.Exec.MaxEvents = 5
        flags.Output.ESDFileName = flags.OnlineEventDisplays.OutputDirectory + "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])
    else:
        flags.Exec.MaxEvents = 20000 # hack until we find a way to fix the memory fragmentation ATEAM-896, this resets the memory after 20k events
        flags.Output.ESDFileName = "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])

    if flags.OnlineEventDisplays.MakeVP1File:
        flags.Output.doWriteESD = True
    else:
        flags.Output.doWriteESD = False

    flags.Output.doJiveXML = False # We call the AlgoJive later on

    if flags.OnlineEventDisplays.OfflineTest:
        flags.Input.Files = ['/eos/home-m/myexley/sharedWithATLASauthors/data25_13p6TeV.00499912.physics_Main.daq.RAW._lb0600._SFO-11._0001.data']
    else:
        flags.Input.Files = [] # Files are read from the ATLAS (or GM test) partition

    flags.Reco.EnableTrigger = False # TODO test True
    flags.Detector.GeometryForward = False
    flags.Detector.EnableFwdRegion = False
    flags.LAr.doHVCorr = False # ATLASRECTS-6823
    flags.DQ.doMonitoring = False
    flags.DQ.doPostProcessing = False
    flags.Concurrency.NumThreads = 1

    if flags.OnlineEventDisplays.MakeVP1File:
        flags.Concurrency.NumThreads = 0 # We cannot run multithread if we want one ESD file per event named with the event number etc. changing the name of the output ESD file when multithreading is not so simple...
        if flags.OnlineEventDisplays.HIMode or flags.OnlineEventDisplays.HIPMode:
            mlog.warning("You have said you want to create a vp1 file 'flags.OnlineEventDisplays.MakeVP1File=True' but you have also said you want to run reconstruction in an heavy ion mode, this will not work as heavy ion mode needs to run in multithreading mode, and in this mode we cannot write out one ESD file (i.e. vp1 file) per event.")
        
    if flags.OnlineEventDisplays.HIMode:
        from AthenaConfiguration.Enums import HIMode
        flags.Reco.HIMode = HIMode.HI
        flags.Concurrency.NumThreads = 1
        flags.OnlineEventDisplays.MakeVP1File = False
        flags.Beam.BunchSpacing = 50

    if flags.OnlineEventDisplays.HIPMode:
        from AthenaConfiguration.Enums import HIMode
        flags.Reco.HIMode = HIMode.HIP
        flags.Concurrency.NumThreads = 1
        flags.OnlineEventDisplays.MakeVP1File = False

    if flags.OnlineEventDisplays.BeamSplashMode:
        flags.Reco.EnableJet=False
        flags.Reco.EnableMet=False
        flags.Reco.EnableTau=False
        flags.Reco.EnablePFlow=False
        flags.Reco.EnableBTagging=False
        flags.Reco.EnableEgamma=False
        flags.Reco.EnableCombinedMuon=False

    from AthenaCommon.Constants import INFO
    flags.Exec.OutputLevel = INFO

    flags.Common.isOnline = not flags.OnlineEventDisplays.OfflineTest

    if flags.OnlineEventDisplays.PartitionName == 'ATLAS' and not flags.OnlineEventDisplays.OfflineTest:
        # For beam plashes when LAr running in a different samples mode, the current run number to LAr config is needed
        run_number = GetRunNumber(flags.OnlineEventDisplays.PartitionName)
        flags.Input.OverrideRunNumber = True
        flags.Input.RunNumbers = [run_number]

        # Get the B field
        (solenoidOn,toroidOn)=GetBFields()
        flags.BField.solenoidOn = solenoidOn
        flags.BField.barrelToroidOn = toroidOn
        flags.BField.endcapToroidOn = toroidOn

    # GM test partition needs to be given the below info
    if (flags.OnlineEventDisplays.PartitionName == 'GMTestPartition' or flags.OnlineEventDisplays.PartitionName == 'GMTestPartitionT9'):
        flags.Input.RunNumbers = [482485] # keep this number the same as (or close to) the run number of the file you are testing on
        flags.Input.LumiBlockNumbers = [111]

    from AthenaConfiguration.Enums import BeamType
    if not flags.OnlineEventDisplays.OfflineTest:
        if flags.OnlineEventDisplays.CosmicMode:
            flags.Beam.Type = BeamType.Cosmics
        else:
            flags.Beam.Type = BeamType.Collisions
    flags.lock()
    flags.dump()
    ##----------------------------------------------------------------------##
    # Call the reconstruction
    from RecJobTransforms.RecoSteering import RecoSteering
    cfg = RecoSteering(flags)

    from IOVDbSvc.IOVDbSvcConfig import addOverride
    if not flags.OnlineEventDisplays.OfflineTest:
        cfg.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2", db=""))

    # Get the input files from the partition
    if not flags.OnlineEventDisplays.OfflineTest:
        from EventDisplaysOnline.ByteStreamConfig import ByteStreamCfg
        cfg.merge(ByteStreamCfg(flags, **kwargs))

    from EventDisplaysOnline.OnlineEventDisplaysSvcConfig import OnlineEventDisplaysSvcCfg
    cfg.merge(OnlineEventDisplaysSvcCfg(flags, **kwargs))

    from JiveXML.OnlineStreamToFileConfig import OnlineStreamToFileCfg
    streamToFileTool = cfg.popToolsAndMerge(OnlineStreamToFileCfg(flags, **kwargs))

    streamToServerTool = None
    if not flags.OnlineEventDisplays.OfflineTest:
        from JiveXML.OnlineStreamToServerConfig import OnlineStreamToServerCfg
        streamToServerTool = cfg.popToolsAndMerge(OnlineStreamToServerCfg(flags, **kwargs))

    from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
    cfg.merge(AlgoJiveXMLCfg(flags,
                             StreamToFileTool = streamToFileTool,
                             StreamToServerTool = streamToServerTool,
                             OnlineMode = not flags.OnlineEventDisplays.OfflineTest))

    if flags.OnlineEventDisplays.HIMode:
        from EventDisplaysOnline.ContainerKeysCfg import getHIContainterKeys
        getHIContainterKeys(cfg)
        
    if flags.OnlineEventDisplays.HIPMode:
        from EventDisplaysOnline.ContainerKeysCfg import getHIPContainterKeys
        getHIPContainterKeys(cfg)
        
    if flags.OnlineEventDisplays.MakeVP1File:
        # This gets the ESD output file which is renamed and moved to the desired output stream
        # in the VP1 Event Prod alg, this creates one ESD file per event
        from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
        cfg.merge(OutputStreamSequencerSvcCfg(flags,incidentName="EndEvent"))
        from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
        streamESD = cfg.getEventAlgo(outputStreamName("ESD"))
        streamESD.OutputFile=flags.Output.ESDFileName
    
        from VP1AlgsEventProd.VP1AlgsEventProdConfig import VP1AlgsEventProdCfg
        cfg.merge(VP1AlgsEventProdCfg(flags, streamESD, **kwargs))

    # switch of the NSW segment making as it takes too much CPU in beamsplashes
    if flags.OnlineEventDisplays.BeamSplashMode:
        cfg.getEventAlgo("MuonSegmentMaker").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker").doMMSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doMMSegments=False
        cfg.dropEventAlgo("QuadNSW_MuonSegmentCnvAlg")

    if flags.OnlineEventDisplays.HorizontalMuonsMode:
        from MuonConfig.MuonReconstructionConfig import MuonNCBTrackCfg
        cfg.merge(MuonNCBTrackCfg(flags))

    cfg.getService("PoolSvc").WriteCatalog = "xmlcatalog_file:PoolFileCatalog_%s_%s.xml" % (jobId[3], jobId[4])

    ##----------------------------------------------------------------------##
    ## Need line below to fix error that occurs when trying to              ##
    ## get CaloRec::ToolConstants/H1WeightsCone4Topo DataObject             ##
    ##----------------------------------------------------------------------##
    cfg.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/det/dqm/GlobalMonitoring/PoolFileCatalog_M7/PoolFileCatalog.xml"]

    # Dump the pickle file
    with open("OnlineEventDisplays.pkl", "wb") as f:
        cfg.store(f)

    return cfg

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.OnlineEventDisplays.HorizontalMuonsMode = False
    flags.OnlineEventDisplays.MakeVP1File = True
    flags.OnlineEventDisplays.CosmicMode = False
    flags.OnlineEventDisplays.HIMode = False
    flags.OnlineEventDisplays.HIPMode = False
    flags.OnlineEventDisplays.BeamSplashMode = False
    flags.OnlineEventDisplays.OfflineTest = False

    cfg = EventDisplaysOnlineCfg(flags)
    # Execute
    sc = cfg.run()
    import sys
    sys.exit(0 if sc.isSuccess() else 1)
