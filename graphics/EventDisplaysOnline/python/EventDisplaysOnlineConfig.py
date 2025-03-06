# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def EventDisplaysOnlineCfg(flags, **kwargs):
    from EventDisplaysOnline.EventDisplaysOnlineHelpers import GetBFields, WaitForPartition, GetUniqueJobID, GetRunNumber

    if not flags.OnlineEventDisplays.OfflineTest:
        from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
        autoConfigOnlineRecoFlags(flags, flags.OnlineEventDisplays.PartitionName)

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
    ## /det/dqm/GlobalMonitoring/GMTestPartition_oks/tdaq-11-02-01/         ##
    ## without_gatherer/GMTestPartitionT9.data.xml                          ##
    ## and in:                                                              ##
    ## /det/dqm/GlobalMonitoring/GMTestPartition_oks/tdaq-11-02-01/         ##
    ## without_gatherer/GMTestPartition.data.xml                            ##
    ##----------------------------------------------------------------------##
    flags.OnlineEventDisplays.PartitionName='ATLAS' # 'ATLAS', 'GMTestPartition' or 'GMTestPartitionT9'

    if flags.OnlineEventDisplays.HIMode:
        flags.OnlineEventDisplays.MaxEvents=200
        flags.OnlineEventDisplays.ProjectTag='data24_hi'
        flags.OnlineEventDisplays.PublicStreams=['HardProbes']
    if flags.OnlineEventDisplays.CosmicMode:
        flags.OnlineEventDisplays.MaxEvents=200
        flags.OnlineEventDisplays.ProjectTag='data24_cos'
        flags.OnlineEventDisplays.PublicStreams=['']
    if flags.OnlineEventDisplays.BeamSplashMode:
        flags.OnlineEventDisplays.MaxEvents=-1 # keep all the events
        flags.OnlineEventDisplays.ProjectTag='data24_13p6TeV'
        flags.OnlineEventDisplays.PublicStreams=['']
    else:
        flags.OnlineEventDisplays.MaxEvents=50
        flags.OnlineEventDisplays.ProjectTag='data24_13p6TeV'
        flags.OnlineEventDisplays.PublicStreams=['Main']

    # Pause this thread until the partition is up
    if not flags.OnlineEventDisplays.OfflineTest:
        WaitForPartition(flags.OnlineEventDisplays.PartitionName)

    if not flags.OnlineEventDisplays.OfflineTest:
        import os
        IPC_timeout = int(os.environ['TDAQ_IPC_TIMEOUT'])
        print(" IPC_timeout Envrionment Variable = %d" %IPC_timeout)

    # Conditions tag
    flags.IOVDb.DatabaseInstance = "CONDBR2"
    if flags.OnlineEventDisplays.OfflineTest:
        flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2024-03'
    else:
        flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2024-02' # Online conditions tag

    # Geometry tag
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'

    if flags.OnlineEventDisplays.HIMode:
        flags.Beam.BunchSpacing = 100 # ns

    flags.Trigger.triggerConfig='DB'

    jobId = GetUniqueJobID()

    # Test wth a small amount of events and write out to e.g. a tmp dir
    if flags.OnlineEventDisplays.PartitionName != 'ATLAS' or flags.OnlineEventDisplays.OfflineTest:
        flags.Exec.MaxEvents = 5
        flags.Output.ESDFileName = flags.OnlineEventDisplays.OutputDirectory + "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])
    else:
        flags.Exec.MaxEvents = 20000 # hack until we find a way to fix the memory fragmentation ATEAM-896, this resets the memory after 20k events
        flags.Output.ESDFileName = "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])

    flags.Output.doWriteESD = True
    flags.Output.doJiveXML = False #we call the AlgoJive later on

    if flags.OnlineEventDisplays.OfflineTest:
        flags.Input.Files = ['/eos/home-m/myexley/sharedWithATLASauthors/data24_13p6TeV.00482485.physics_Main.daq.RAW._lb0111._SFO-13._0002.data']
    else:
        flags.Input.Files = [] # Files are read from the ATLAS (or GM test) partition

    flags.Reco.EnableTrigger = False # TODO test True
    flags.Detector.GeometryForward = False
    flags.Detector.EnableFwdRegion = False
    flags.LAr.doHVCorr = False # ATLASRECTS-6823

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
    flags.Concurrency.NumThreads = 0

    flags.Common.isOnline = not flags.OnlineEventDisplays.OfflineTest

    if flags.OnlineEventDisplays.PartitionName == 'ATLAS' and not flags.OnlineEventDisplays.OfflineTest:
        # For beam plashes when LAr running in a different samples mode, the current run number to LAr config is needed
        run_number = GetRunNumber(flags.OnlineEventDisplays.PartitionName)
        flags.Input.OverrideRunNumber = True
        flags.Input.RunNumbers = [run_number]

        # Get the B field
        (solenoidOn,toroidOn)=GetBFields()
        flags.BField.override = True
        flags.BField.solenoidOn = solenoidOn
        flags.BField.barrelToroidOn = toroidOn
        flags.BField.endcapToroidOn = toroidOn

    # GM test partition needs to be given the below info
    if (flags.OnlineEventDisplays.PartitionName == 'GMTestPartition' or flags.OnlineEventDisplays.PartitionName == 'GMTestPartitionT9'):
        flags.Input.OverrideRunNumber = True
        flags.Input.RunNumbers = [454188] # keep this number the same as (or close to) the run number of the file you are testing on
        flags.Input.LumiBlockNumbers = [1]
        flags.Input.ProjectName = flags.OnlineEventDisplays.ProjectTag

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

    # This creates an ESD file per event which is renamed and moved to the desired output
    # dir in the VP1 Event Prod alg
    from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
    cfg.merge(OutputStreamSequencerSvcCfg(flags,incidentName="EndEvent"))
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
    streamESD = cfg.getEventAlgo(outputStreamName("ESD"))

    from VP1AlgsEventProd.VP1AlgsEventProdConfig import VP1AlgsEventProdCfg
    cfg.merge(VP1AlgsEventProdCfg(flags, streamESD, **kwargs))

    # switch of the NSW segment making as it takes too much CPU in beamsplashes
    if flags.OnlineEventDisplays.BeamSplashMode:
        cfg.getEventAlgo("MuonSegmentMaker").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker").doMMSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doMMSegments=False
        cfg.dropEventAlgo("QuadNSW_MuonSegmentCnvAlg")

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

    flags.OnlineEventDisplays.CosmicMode = False
    flags.OnlineEventDisplays.HIMode = False
    flags.OnlineEventDisplays.BeamSplashMode = False
    flags.OnlineEventDisplays.OfflineTest = False

    cfg = EventDisplaysOnlineCfg(flags)
    # Execute
    sc = cfg.run()
    import sys
    sys.exit(0 if sc.isSuccess() else 1)
