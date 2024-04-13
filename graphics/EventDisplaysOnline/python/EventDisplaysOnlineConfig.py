# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from EventDisplaysOnline.EventDisplaysOnlineHelpers import GetBFields,WaitForPartition
    from AthenaConfiguration.Enums import BeamType

    isCosmicData = False
    isHIMode = False #TODO
    isBeamSplashMode = False
    isOfflineTest = True
    testWithoutPartition = True
    HorizontalMuons_quickReco = False

    # An explicit list for nominal data taking to exclude some high rate streams
    # Empty list to read all
    streamsWanted = ['MinBias','express','ZeroBias','CosmicCalo','IDCosmic','CosmicMuons','Background','Standby','L1Calo','Main']
    if isBeamSplashMode:
        streamsWanted = ['MinBias'] #if trigger fails it will go to debug_HltError

    # If testing at p1, create dir /tmp/your_user_name and write out to /tmp/your_user_name to see output
    outputDirectory="/atlas/EventDisplayEvents/"

    if isOfflineTest:
        outputDirectory="."

    sendToPublicStream = False # Gets set later, overwrite here to True to test it

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
    partitionName = 'ATLAS' # 'ATLAS', 'GMTestPartition' or 'GMTestPartitionT9'

    if isHIMode:
        maxEvents=200 # Number of events to keep per stream in /atlas/EventDisplays/stream
        projectTags=['data24_hi']
        projectName='data24_hi'
        publicStreams=['HardProbes']
    if isCosmicData:
        maxEvents=200
        projectTags=['data24_cos']
        projectName='data24_cos'
        publicStreams=['']
    if isBeamSplashMode:
        maxEvents=-1
        projectTags=['data24_13p6TeV']
        projectName='data24_13p6TeV'
        publicStreams=['']
    else:
        maxEvents=100
        projectTags=['data24_13p6TeV']
        projectName='data24_13p6TeV'
        publicStreams=['Main']

    # Pause this thread until the partition is up
    if not testWithoutPartition or not isOfflineTest:
        WaitForPartition(partitionName)

    # Setup unique output files (so that multiple Athena jobs on the same machine don't interfere)
    import os
    jobId = os.environ.get('TDAQ_APPLICATION_NAME', '').split(':')
    if not len(jobId) == 5:
        from random import randint
        jobId = ['Athena-EventProcessor', 'Athena-EventDisplays-Segment', 'EventDisplays-Rack', 'tmp', '%d' % randint(0, 999)]

    if not isOfflineTest:
        IPC_timeout = int(os.environ['TDAQ_IPC_TIMEOUT'])
        print(" IPC_timeout Envrionment Variable = %d" %IPC_timeout)

    ##----------------------------------------------------------------------##
    # Define the flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    if not isOfflineTest:
        from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
        autoConfigOnlineRecoFlags(flags, partitionName)

    # Conditions tag
    flags.IOVDb.DatabaseInstance = "CONDBR2"
    if isOfflineTest:
        flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-02'
    else:
        flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01' # Online conditions tag

    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00' # Geometry tag

    if isHIMode:
        flags.Beam.BunchSpacing = 100 # ns

    flags.Trigger.triggerConfig='DB'

    # Test wth a small amount of events and write out to e.g. a tmp dir
    if testWithoutPartition or partitionName != 'ATLAS' or isOfflineTest:
        flags.Exec.MaxEvents = 3
        flags.Output.ESDFileName = outputDirectory + "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])
    else:
        flags.Exec.MaxEvents = 20000 # hack until we find a way to fix the memory fragmentation ATEAM-896, this resets the memory after 20k events
        flags.Output.ESDFileName = "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])

    flags.Output.doWriteESD = True
    #flags.Output.doJiveXML = False #we call the AlgoJive later on

    if isOfflineTest:
        flags.Input.Files = ['/eos/home-m/myexley/sharedWithATLASauthors/data23_13p6TeV.00454188.physics_Main.daq.RAW._lb0633._SFO-12._0002.data']
    else:
        flags.Input.Files = [] # Files are read from the ATLAS (or GM test) partition

    flags.Reco.EnableTrigger = False # TODO test True
    flags.Detector.GeometryForward = False
    flags.Detector.EnableFwdRegion = False
    flags.LAr.doHVCorr = False # ATLASRECTS-6823
    
    if isBeamSplashMode or HorizontalMuons_quickReco:
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

    flags.Common.isOnline = not isOfflineTest

    if partitionName == 'ATLAS' and not testWithoutPartition and not isOfflineTest:
        # Read run number from the partition
        # For beam plashes when LAr running in 32 samples mode, the current run number to LAr config is needed
        # IS stands for Information Service
        from ispy import ISObject, IPCPartition, ISInfoAny, ISInfoDictionary
        part = IPCPartition(partitionName)
        RunParams = ISObject(part, 'RunParams.RunParams', 'RunParams')
        RunParams.checkout()
        run_number = RunParams.getAttributeValue('run_number')
        flags.Input.OverrideRunNumber = True
        flags.Input.RunNumbers = [run_number]

        # Is the data allowed to be seen by the general public on atlas live
        ready4physics = ISInfoAny()
        ISInfoDictionary(part).getValue('RunParams.Ready4Physics', ready4physics)
        print("Ready for physics: %s " % ready4physics.get())
        physicsReady = ISObject(part, 'RunParams.Ready4Physics','Ready4PhysicsInfo')
        physicsReady.checkout()
        print("Ready for physics: %r" % (physicsReady.ready4physics))
        if physicsReady.ready4physics and RunParams.T0_project_tag in projectTags:
            sendToPublicStream = True

        # Get the B field
        (solenoidOn,toroidOn)=GetBFields()
        flags.BField.override = True
        flags.BField.solenoidOn = solenoidOn
        flags.BField.barrelToroidOn = toroidOn
        flags.BField.endcapToroidOn = toroidOn

    # GM test partition needs to be given the below info
    if (partitionName == 'GMTestPartition' or partitionName == 'GMTestPartitionT9'):
        flags.Input.OverrideRunNumber = True
        flags.Input.RunNumbers = [454188] # keep this number the same as (or close to) the run number of the file you are testing on
        flags.Input.LumiBlockNumbers = [1]
        flags.Input.ProjectName = projectName

    if not testWithoutPartition:
        if isCosmicData:
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
    if not isOfflineTest:
        cfg.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2", db=""))

    # get the input files from the partition
    if not testWithoutPartition:
        from EventDisplaysOnline.ByteStreamConfig import ByteStreamCfg
        cfg.merge(ByteStreamCfg(flags, partitionName, streamsWanted, isBeamSplashMode))

    from EventDisplaysOnline.OnlineEventDisplaysSvcConfig import OnlineEventDisplaysSvcCfg
    cfg.merge(OnlineEventDisplaysSvcCfg(flags, maxEvents, outputDirectory, sendToPublicStream, publicStreams, streamsWanted, isBeamSplashMode))
    onlineEventDisplaysSvc = cfg.getService("OnlineEventDisplaysSvc")

    from JiveXML.OnlineStreamToFileConfig import OnlineStreamToFileCfg
    streamToFileTool = cfg.popToolsAndMerge(OnlineStreamToFileCfg(flags, OnlineEventDisplaysSvc = onlineEventDisplaysSvc))

    streamToServerTool = None
    if not isOfflineTest:
        from JiveXML.OnlineStreamToServerConfig import OnlineStreamToServerCfg
        streamToServerTool = cfg.popToolsAndMerge(OnlineStreamToServerCfg(flags, OnlineEventDisplaysSvc = onlineEventDisplaysSvc))

    from AthenaCommon.Constants import DEBUG
    from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
    cfg.merge(AlgoJiveXMLCfg(flags,
                             StreamToFileTool = streamToFileTool,
                             StreamToServerTool = streamToServerTool,
                             OnlineMode = not isOfflineTest,
                             OutputLevel = DEBUG))

    # This creates an ESD file per event which is renamed and moved to the desired output
    # dir in the VP1 Event Prod alg
    from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
    cfg.merge(OutputStreamSequencerSvcCfg(flags,incidentName="EndEvent"))
    streamESD = cfg.getEventAlgo("OutputStreamESD")

    from VP1AlgsEventProd.VP1AlgsEventProdConfig import VP1AlgsEventProdCfg
    cfg.merge(VP1AlgsEventProdCfg(flags, streamESD, OnlineEventDisplaysSvc = onlineEventDisplaysSvc))

    # switch of the NSW segment making as it takes too much CPU in beamsplashes
    if isBeamSplashMode or HorizontalMuons_quickReco:
        cfg.getEventAlgo("MuonSegmentMaker").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker").doMMSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doStgcSegments=False
        cfg.getEventAlgo("MuonSegmentMaker_NCB").doMMSegments=False
        cfg.dropEventAlgo("QuadNSW_MuonSegmentCnvAlg")

    if isBeamSplashMode:
        cfg.getPublicTool("CaloLArRetriever").LArlCellThreshold=500.
        cfg.getPublicTool("CaloHECRetriever").HEClCellThreshold=500.

    cfg.getService("PoolSvc").WriteCatalog = "xmlcatalog_file:PoolFileCatalog_%s_%s.xml" % (jobId[3], jobId[4])

    ##----------------------------------------------------------------------##
    ## Need line below to fix error that occurs when trying to              ##
    ## get CaloRec::ToolConstants/H1WeightsCone4Topo DataObject             ##
    ##----------------------------------------------------------------------##
    cfg.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/det/dqm/GlobalMonitoring/PoolFileCatalog_M7/PoolFileCatalog.xml"]

    # Dump the pickle file
    with open("OnlineEventDisplays.pkl", "wb") as f:
        cfg.store(f)

    # Execute
    sc = cfg.run()
    import sys
    sys.exit(0 if sc.isSuccess() else 1)
