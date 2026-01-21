#!/usr/bin/env python3
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
"""
@author Salah-Eddine Dahbi, Song-Ming Wang
@brief Global Monitoring job option to run online DQ with
       new-style configuration.
-- Offline on lxplus for ART test :
   --- python GlobalMonitoring.py IOVDb.GlobalTag="conditions_Tag" --offline
        --filesInput [path_to_RAW_data_file] --evtMax [Max_number_events]
   --- note: if the filesInput is not provided a defaultTestFile will be used.
   --- note: if the evtMax is not provided, the default is 100.
-- Online is the default: python GlobalMonitoring.py"
"""

if __name__ == '__main__':
    import sys
    import os
    from AthenaConfiguration.Enums import BeamType
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import Format
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--offline', action='store_true',
                        help="Run in offline mode (default is online)")
    # change default
    parser.set_defaults(threads=1)
    args, _ = parser.parse_known_args()
    is_online = not args.offline
    flags.fillFromArgs(parser=parser)

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    if args.loglevel:
        log.setLevel(getattr(Constants, args.loglevel))
    if is_online:
        print("Running in online mode")
        partitionName = os.environ.get("TDAQ_PARTITION", "ATLAS")
        from AthenaConfiguration.AutoConfigOnlineRecoFlags import (
            autoConfigOnlineRecoFlags)
        autoConfigOnlineRecoFlags(flags, partitionName)

        flags.Concurrency.NumThreads = 1
        flags.Exec.MaxEvents = -1

        # Set up partition for online monitoring
        from ispy import IPCPartition, ISObject
        p = IPCPartition(partitionName)
        if not p.isValid():
            print("Partition:", p.name(), "is not valid")
            sys.exit(1)

        if partitionName == "ATLAS" or partitionName == 'ATLAS_MP1':
            try:
                y = ISObject(p, 'RunParams.RunParams', 'RunParams')
            except Exception:
                print("Could not find Run Parameters in IS - "
                      "Set default beam type to 'cosmics'")
                beamType = BeamType.Cosmics
            else:
                y.checkout()
                beamtype = y.beam_type
                beamenergy = y.beam_energy
                runnumber = y.run_number
                project = y.T0_project_tag
                print("RUN CONFIGURATION: beam type %i, beam energy %i, "
                      "run number %i, project tag %s" %
                      (beamtype, beamenergy, runnumber, project))
            # Define beam type based on project tag name
            if project[7:10] == "cos":
                beamType = BeamType.Cosmics
            else:
                beamType = BeamType.Collisions
        else:
            beamType = BeamType.Collisions

        # Online specific flags setup
        flags.DQ.useTrigger = True
        flags.DQ.triggerDataAvailable = True
        flags.DQ.enableLumiAccess = False
        print("Online mode setup complete.")
    # Offline mode logic
    else:
        print("Running in offline mode")
        partitionName = 'GMTestPartition'

        # Offline specific settings
        flags.DQ.useTrigger = False
        flags.Reco.EnableTrigger = False
        flags.DQ.triggerDataAvailable = False
        flags.LAr.doHVCorr = False
        flags.DQ.enableLumiAccess = True
        isOnlineStateless = True
        isOfflineTest = True
        print("Offline mode setup complete.")

    # flags.DQ.enableLumiAccess
    # for ART Test on LXPLUS needs to be True, for online/test partition
    # it need to be false with applying changes to CaloMonitoringConfig.py
    # Jira "ATLASRECTS-8077"

    # ############################################################
    # COND tag and GEO are needed
    # for running over a test partition online
    # ############################################################
    detDescrVersion = 'ATLAS-R3S-2021-03-02-00'
    flags.IOVDb.DatabaseInstance = "CONDBR2"
    flags.GeoModel.Layout = "atlas"
    flags.GeoModel.AtlasVersion = detDescrVersion
    # flags.IOVDb.GlobalTag="CONDBR2-HLTP-2024-02"
    flags.Trigger.triggerConfig = 'DB'

    if partitionName not in ['ATLAS', 'ATLAS_MP1']:
        from AthenaConfiguration.TestDefaults import defaultTestFiles
        # if --filesInput was specified as well (!) this will override
        if not args.filesInput and args.offline:
            flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        # if --evtMax was specified as well this will override
        if not args.evtMax:
            flags.Exec.MaxEvents = 100

    if flags.Input.Format is Format.BS:
        if flags.DQ.Environment not in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading RAW file, but DQ.Environment set to %s',
                        flags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')
        log.info('Will schedule reconstruction, as best we know')
    else:
        if flags.DQ.Environment in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading POOL file, but DQ.Environment set to %s',
                        flags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')

    flags.Common.isOnline = True

    print("=" * 50)
    print("Configuration Info".center(50))
    print("=" * 50)
    print(f"{'Partition:':30} {partitionName}")
    print(f"{'Max events:':30} {flags.Exec.MaxEvents}")
    print(f"{'Run numbers:':30} {flags.Input.RunNumbers}")
    print(f"{'Lumi block numbers:':30} {flags.Input.LumiBlockNumbers}")
    print("=" * 50)
    if not args.offline:
        print("=" * 50)
        print("Beam Info".center(50))
        print(f"{'Beam Energy:':30} {flags.Beam.Energy}")
        print(f"{'Beam Type:':30} {flags.Beam.Type}")
        print(f"{'Beam Number Of Collisions:':30} "
              f"{flags.Beam.NumberOfCollisions}")
        print(f"{'Beam Estimated Luminosity:':30} "
              f"{flags.Beam.estimatedLuminosity}")
        print(f"{'Beam Bunch Spacing:':30} {flags.Beam.BunchSpacing}")
        print("=" * 50)

    flags.Input.isMC = False

    flags.InDet.useSctDCS = False
    flags.InDet.useDCS = False

    flags.DQ.doMonitoring = True
    flags.DQ.doPostProcessing = True
    flags.DQ.FileKey = ''
    flags.DQ.Environment = 'online'
    flags.Output.HISTFileName = 'monitoring.root'
    flags.LAr.doHVCorr = False
    flags.Detector.EnableCalo = True
    # ############################################################
    # Set monitoring flags
    # ############################################################
    flags.DQ.Steering.doGlobalMon = False
    # JetInputsMon
    flags.DQ.Steering.doHLTMon = False
    # doZDCmon
    if '_hi' in flags.Input.ProjectName:
        flags.DQ.Steering.doZDCMon = True
        flags.Reco.EnableZDC = True
    # LVL1Calo
    flags.DQ.Steering.doLVL1CaloMon = False
    flags.DQ.Steering.doLVL1InterfacesMon = False
    # DataFlowMon - Do we want this? If so, set False
    flags.DQ.Steering.doDataFlowMon = False
    # CTPMon - If we want this in ATLAS partition, set to True
    # under the if statement below
    flags.DQ.Steering.doCTPMon = False
    if partitionName not in ['ATLAS', 'ATLAS_MP1']:
        flags.DQ.useTrigger = False
        flags.DQ.triggerDataAvailable = False
        flags.Reco.EnableTrigger = False

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    if (args.loglevel is None or
            getattr(Constants, args.loglevel) <= Constants.INFO):
        flags.dump()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    acc.addService(CompFactory.ByteStreamCnvSvc(), primary=True)
    # if args.offline:
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    if not args.offline and not args.filesInput:
        from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
        acc.merge(EmonByteStreamCfg(flags,
                                    type_names=['CTP_RDO/CTP_RDO']))
        bytestreamInput = acc.getService("ByteStreamInputSvc")

        # ############################################################
        # The name of the partition you want to connect
        # ############################################################
        bytestreamInput.Partition = partitionName

        bytestreamInput.Key = "dcm"
        bytestreamInput.KeyCount = 40
        bytestreamInput.BufferSize = 120  # event buffersize

        # #######################################################
        # TimeOut (in milliseconds) - Prevent job with low rates
        # to reconnect too often to SFIs
        # ######################################################
        bytestreamInput.Timeout = 600000

        bytestreamInput.UpdatePeriod = 200

        bytestreamInput.StreamNames = ['express']
        bytestreamInput.StreamType = "physics"
        bytestreamInput.StreamLogic = "Or"

        bytestreamInput.PublishName = os.environ.get(
            "TDAQ_APPLICATION_NAME", "GlobalMonitoring")
        # Ak: 26-05-2014 - needed to write out the gathered histograms
        # to the correct server
        bytestreamInput.ISServer = 'Histogramming-Global-iss'
        if partitionName != 'ATLAS' and partitionName != 'ATLAS_MP1':
            bytestreamInput.ISServer = 'Histogramming'

        if partitionName == 'ATLAS' or partitionName == 'ATLAS_MP1':
            from PyUtils.OnlineISConfig import GetAtlasReady
            if beamType == BeamType.Cosmics:
                bytestreamInput.StreamNames = ['express', 'IDCosmic',
                                               'HLT_IDCosmic', 'CosmicMuons',
                                               'CosmicCalo']
            else:
                if GetAtlasReady():
                    # printfunc ("ATLAS READY, reading express stream")
                    bytestreamInput.StreamNames = ['express']
                    bytestreamInput.StreamType = "express"
                else:
                    # printfunc ("ATLAS NOT READY, reading standby stream")
                    bytestreamInput.StreamNames = ['express', 'IDCosmic',
                                                   'HLT_IDCosmic',
                                                   'CosmicMuons', 'MinBias',
                                                   'Standby', 'Main',
                                                   'CosmicCalo']
                    # bytestreamInput.StreamNames = ['MinBias']
        else:
            bytestreamInput.StreamLogic = 'Ignore'
            bytestreamInput.PublishName = 'GMT9_sdahbi_R55'

    from xAODEventInfoCnv.xAODEventInfoCnvConfig import (
        EventInfoBeamSpotDecoratorAlgCfg)
    acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))

    # Calo
    acc.flagPerfmonDomain('Calo')
    if flags.Detector.EnableCalo:
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        acc.merge(CaloRecoCfg(flags))

    # ZDC monitoring
    acc.flagPerfmonDomain('ZDC')
    if flags.Reco.EnableZDC:
        from ZdcRec.ZdcRecConfig import ZdcRecCfg
        acc.merge(ZdcRecCfg(flags))

    # CaloExtension
    acc.flagPerfmonDomain('CaloExtension')
    if flags.Reco.EnableCaloExtension:
        from TrackToCalo.CaloExtensionBuilderAlgCfg import (
            CaloExtensionBuilderCfg)
        acc.merge(CaloExtensionBuilderCfg(flags))
    acc.flagPerfmonDomain('CaloRings')
    if flags.Reco.EnableCaloRinger:
        from CaloRingerAlgs.CaloRingerAlgsConfig import CaloRingerSteeringCfg
        acc.merge(CaloRingerSteeringCfg(flags))

    acc.flagPerfmonDomain('ID')
    if flags.Reco.EnableTracking:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        acc.merge(InDetTrackRecoCfg(flags))
    # Muon
    acc.flagPerfmonDomain('Muon')
    if flags.Detector.EnableMuon:
        from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
        acc.merge(MuonReconstructionCfg(flags))
    # Muon Combined
    acc.flagPerfmonDomain('CombinedMuon')
    if flags.Reco.EnableCombinedMuon:
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import (
            MuonCombinedReconstructionCfg)
        acc.merge(MuonCombinedReconstructionCfg(flags))

    # Jet
    acc.flagPerfmonDomain('Jets')
    if flags.Reco.EnableJet:
        from JetRecConfig.JetRecoSteering import JetRecoSteeringCfg
        acc.merge(JetRecoSteeringCfg(flags))

    # TrackParticleCellAssociation
    # add cells crossed by high pt ID tracks
    acc.flagPerfmonDomain('TrackCellAssociation')
    if flags.Reco.EnableTrackCellAssociation:
        from TrackParticleAssociationAlgs.TrackParticleAssociationAlgsConfig import (  # noqa: E501
            TrackParticleCellAssociationCfg)
        acc.merge(TrackParticleCellAssociationCfg(flags))
    acc.flagPerfmonDomain('Jets')
    if flags.Reco.EnableGlobalFELinking:
        # We also need to build links between the newly
        # created jet constituents (GlobalFE)
        # and electrons,photons,muons and taus
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        acc.merge(PFGlobalFlowElementLinkingCfg(flags))

    acc.flagPerfmonDomain('PFlow')
    if flags.Reco.EnablePFlow:
        from eflowRec.PFRun3Config import PFCfg
        acc.merge(PFCfg(flags))
    # EGamma
    acc.flagPerfmonDomain('EGamma')
    if flags.Reco.EnableEgamma:
        from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
        acc.merge(EGammaSteeringCfg(flags))
    # Tau
    acc.flagPerfmonDomain('Tau')
    if flags.Reco.EnableTau:
        from tauRec.TauConfig import TauReconstructionCfg
        acc.merge(TauReconstructionCfg(flags))

    # Trigger
    acc.flagPerfmonDomain('Trigger')
    if flags.Reco.EnableTrigger:
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
        acc.merge(TriggerRecoCfg(flags))

    # MET
    acc.flagPerfmonDomain('MET')
    if flags.Reco.EnableMet:
        from METReconstruction.METRecCfg import METCfg
        acc.merge(METCfg(flags))

    # Isolation
    acc.flagPerfmonDomain('Isolation')
    if flags.Reco.EnableIsolation:
        from IsolationAlgs.IsolationSteeringConfig import IsolationSteeringCfg
        acc.merge(IsolationSteeringCfg(flags))

    # Lucid
    acc.flagPerfmonDomain('Lucid')
    if flags.Detector.EnableLucid:
        from ForwardRec.LucidRecConfig import LucidRecCfg
        acc.merge(LucidRecCfg(flags))

    # Heavy Ion
    acc.flagPerfmonDomain('HI')
    if flags.Reco.EnableHI:
        from HIRecConfig.HIRecConfig import HIRecCfg
        acc.merge(HIRecCfg(flags))

    # Btagging
    acc.flagPerfmonDomain('FTag')
    if flags.Reco.EnableBTagging:
        from BTagging.BTagConfig import BTagRecoSplitCfg
        acc.merge(BTagRecoSplitCfg(flags))
    # #####################################################
    # Load Monitoring
    # #####################################################
    # if flags.Reco.EnablePostProcessing:
    #     acc.merge(RecoPostProcessingCfg(flags))

    if not args.offline:
        # Monitoring
        acc.flagPerfmonDomain('DQM')
        if flags.DQ.doMonitoring:
            from AthenaMonitoring.AthenaMonitoringCfg import (
                AthenaMonitoringCfg, AthenaMonitoringPostprocessingCfg)
            acc.merge(AthenaMonitoringCfg(flags))
            if flags.DQ.doPostProcessing:
                acc.merge(AthenaMonitoringPostprocessingCfg(flags))

        if flags.DQ.Steering.doInDetMon:
            if flags.DQ.Steering.InDet.doGlobalMon:
                # info('Set up InDet Global monitoring')
                from InDetGlobalMonitoringRun3Test.InDetGlobalMonitoringRun3TestConfig import (  # noqa: E501
                    InDetGlobalMonitoringRun3TestConfig)
                acc.merge(InDetGlobalMonitoringRun3TestConfig(flags))
            if flags.DQ.Steering.InDet.doAlignMon:
                # info('Set up Alignment monitoring')
                from InDetAlignmentMonitoringRun3.InDetAlignmentMonitoringRun3Config import (  # noqa: E501
                    InDetAlignmentMonitoringRun3Config)
                acc.merge(InDetAlignmentMonitoringRun3Config(flags))
        if flags.DQ.Steering.doPixelMon:
            from PixelMonitoring.PixelMonitoringConfig import (
                PixelMonitoringConfig)
            acc.merge(PixelMonitoringConfig(flags))

        if flags.DQ.Steering.doSCTMon:
            # info('Set up SCT monitoring')
            from SCT_Monitoring.SCTMonitoringConfig import SCTMonitoringConfig
            acc.merge(SCTMonitoringConfig(flags))

        if flags.DQ.Steering.doTRTMon:
            # info('Set up TRT monitoring')
            from TRTMonitoringRun3.TRTMonitoringRun3Config import (
                TRTMonitoringRun3Cfg)
            acc.merge(TRTMonitoringRun3Cfg(flags))

        if flags.DQ.Steering.doLArMon:
            # info('Set up LAr monitoring')
            from LArConfiguration.LArMonitoringConfig import (
                LArMonitoringConfig)
            acc.merge(LArMonitoringConfig(flags))

        if flags.DQ.Steering.doTileMon:
            # info('Set up Tile monitoring')
            from TileMonitoring.TileMonitoringConfig import TileMonitoringCfg
            acc.merge(TileMonitoringCfg(flags))

        if flags.DQ.Steering.doCaloGlobalMon:
            # info('Set up Calo monitoring')
            from CaloMonitoring.CaloMonitoringConfig import CaloMonitoringCfg
            acc.merge(CaloMonitoringCfg(flags))

        if flags.DQ.Steering.doMuonMon:
            # info('Set up Muon monitoring')
            from MuonDQAMonitoring.MuonDQAMonitoringConfig import (
                MuonDQAMonitoringConfig)
            acc.merge(MuonDQAMonitoringConfig(flags))

        if flags.DQ.Steering.doHLTMon:
            # info('Set up HLT monitoring')
            from TrigHLTMonitoring.TrigHLTMonitorAlgorithm import (
                TrigHLTMonTopConfig)
            acc.merge(TrigHLTMonTopConfig(flags))

        if flags.DQ.Steering.doJetTagMon:
            # info('Set up JetTagging monitoring')
            from JetTagMonitoring.JetTagMonitorAlgorithm import (
                JetTagMonitorConfig)
            acc.merge(JetTagMonitorConfig(flags))

        if flags.DQ.Steering.doEgammaMon:
            # info('Set up Egamma monitoring')
            from egammaPerformance.egammaMonitoringConfig import (
                egammaMonitoringConfig)
            acc.merge(egammaMonitoringConfig(flags))

        if flags.DQ.Steering.doJetMon:
            # info('Set up Jet monitoring')
            from JetMonitoring.JetMonitoringStandard import (
                standardJetMonitoring)
            acc.merge(standardJetMonitoring(flags))
            # Need to create links between global FE, created in jet finding,
            # and other objects
            # MET monitoring will need these in some workflows
            # (but not in tier0ESD)
            if flags.DQ.Environment != 'tier0ESD':
                # Only run PFlow linking for ion runs in UPC mode
                if (not flags.Reco.EnableHI or
                        (flags.Reco.EnableHI and flags.Tracking.doUPC)):
                    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
                    acc.merge(PFGlobalFlowElementLinkingCfg(flags))

        # Set TRT expert flags
        acc.getEventAlgo("AlgTRTMonitoringRun3RAW").doExpert = True

    from IOVDbSvc.IOVDbSvcConfig import addOverride
    acc.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN",
                          "TRTCalibPID_NN_v2", db=""))

    # Need to add this line since it was needed as explained in the below link.
    # Otherwise jobs crash
    # https://gitlab.cern.ch/atlas/athena/-/blob/master/Reconstruction/
    # RecExample/RecExOnline/share/RecExOnline_postconfig.py#L12
    acc.getService("PoolSvc").ReadCatalog += [
        "xmlcatalog_file:/det/dqm/GlobalMonitoring/"
        "PoolFileCatalog_M7/PoolFileCatalog.xml"]

    # Run
    sc = acc.run()
    sys.exit(0 if sc.isSuccess() else 1)
