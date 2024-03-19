#!/usr/bin/env python3
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@author Tulin Mete
@brief Global Monitoring job option to run online DQ with new-style configuration. 
Include parts from  StateLessPT_NewConfig.py and RecoSteering.py
'''

if __name__=='__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import Format
    flags = initConfigFlags()

    # ############################################################
    # Get partition name
    # ############################################################
    import os
    partitionName = os.environ.get("TDAQ_PARTITION", "ATLAS")

    from ispy import IPCPartition, ISObject

    # ################################
    # To read run parameters from IS
    # ################################
    p = IPCPartition(partitionName)
    if not p.isValid():
        print("Partition:",p.name(),"is not valid")
        sys.exit(1)
   
    ### ATLAS partition: Read Global Run Parameters to configure the jobs
    from AthenaConfiguration.Enums import BeamType
  
    if partitionName == "ATLAS" or partitionName == 'ATLAS_MP1':
        try:
            y = ISObject(p, 'RunParams.RunParams', 'RunParams')
        except:
            print("Could not find Run Parameters in IS - Set default beam type to 'cosmics'")
            beamType=BeamType.Cosmics
        else:
            y.checkout()
            beamtype = y.beam_type
            beamenergy = y.beam_energy
            runnumber = y.run_number
            project = y.T0_project_tag
            print("RUN CONFIGURATION: beam type %i, beam energy %i, run number %i, project tag %s"%(beamtype,beamenergy,runnumber,project))    
        # Define beam type based on project tag name
        if project[7:10]=="cos":
               beamType=BeamType.Cosmics
        else:
            beamType=BeamType.Collisions
               
    ### TEST partition
    else:
        beamType=BeamType.Collisions

    # ############################################################
    # COND tag and GEO are needed 
    # for running over a test partition online 
    # ############################################################
    conditionsTag = 'CONDBR2-HLTP-2022-02'
    detDescrVersion = 'ATLAS-R3S-2021-03-00-00'   

    # ############################################################
    # Setting flags
    # ############################################################
    from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
    autoConfigOnlineRecoFlags(flags, partitionName) 

    flags.Concurrency.NumThreads = 1    

    flags.Exec.MaxEvents = -1

    flags.Beam.Type = beamType 
    flags.Beam.BunchSpacing = 25
    print("RUN CONFIGURATION: Beamtype =",flags.Beam.Type)
   
    if partitionName != 'ATLAS' and partitionName != 'ATLAS_MP1': 
        flags.Input.ProjectName = 'data21_900GeV' 
        flags.Exec.MaxEvents = 20
        flags.Input.RunNumbers = [431894]
        flags.Input.LumiBlockNumbers = [1] 

    flags.Common.isOnline = True

    flags.Input.Format = Format.BS
    flags.Input.isMC = False

    flags.IOVDb.DatabaseInstance = "CONDBR2"
    flags.IOVDb.GlobalTag = conditionsTag

    flags.GeoModel.Layout="atlas"
    flags.GeoModel.AtlasVersion = detDescrVersion

    flags.Trigger.triggerConfig='DB'

    flags.InDet.useSctDCS = False

    flags.DQ.enableLumiAccess = False   
    flags.DQ.doMonitoring = True
    flags.DQ.doPostProcessing = True
    flags.DQ.useTrigger = True
    flags.DQ.triggerDataAvailable = True
    flags.DQ.FileKey = ''
    flags.DQ.Environment = 'online'
    
    flags.LAr.doHVCorr = False

    if (partitionName != 'ATLAS' and partitionName != 'ATLAS_MP1'):
        flags.DQ.useTrigger = False
        flags.DQ.triggerDataAvailable = False    

    # ############################################################
    # Set reco flags
    # ############################################################
    flags.Detector.EnableCalo = True
    flags.Reco.EnableTracking = True
    flags.Detector.EnableMuon = True
    flags.Reco.EnableCombinedMuon = True
    flags.Reco.EnableJet = True
    flags.Reco.EnablePFlow = True
    flags.Reco.EnableEgamma = True
    flags.Reco.EnableTrigger = True
    
    if (partitionName != 'ATLAS' and partitionName != 'ATLAS_MP1'):
        flags.Reco.EnableTrigger = False

    flags.Reco.EnableMet = True
    flags.Reco.EnableIsolation = True
    flags.Detector.EnableLucid  = True
    flags.Reco.EnableHI = False
    flags.Reco.EnableBTagging = True

    # ############################################################
    # Set monitoring flags
    # ############################################################
    ### CaloMon
    flags.DQ.Steering.doCaloGlobalMon = True
    ### IDMon
    flags.DQ.Steering.doInDetMon = True
    flags.DQ.Steering.InDet.doGlobalMon = True
    flags.DQ.Steering.InDet.doAlignMon = True
    ### LArMon
    flags.DQ.Steering.doLArMon = True
    ### TileMon
    flags.DQ.Steering.doTileMon = True
    ### JetMon
    flags.DQ.Steering.doJetMon = True
    ### PixelMon
    flags.DQ.Steering.doPixelMon = True
    ### SCTMon
    flags.DQ.Steering.doSCTMon = False # Need to set this to True when ATLASRECTS-7652 is resolved
    ### TRTMon
    flags.DQ.Steering.doTRTMon = True
    ### MuonMon
    flags.DQ.Steering.doMuonMon = True
    ### PhysMon
    flags.DQ.Steering.doMissingEtMon = True
    flags.DQ.Steering.doEgammaMon = True
    flags.DQ.Steering.doJetTagMon = True
    flags.DQ.Steering.doTauMon = True
    flags.DQ.Steering.doGlobalMon = False
    ### JetInputsMon
    flags.DQ.Steering.doJetInputsMon = True
    ### HLTMon
    flags.DQ.Steering.doHLTMon = True
    ### AFPmon
    flags.DQ.Steering.doAFPMon = True
    ### LVL1Calo
    flags.DQ.Steering.doLVL1CaloMon = False
    flags.DQ.Steering.doLVL1InterfacesMon = False 
    ### DataFlowMon
    flags.DQ.Steering.doDataFlowMon = False
    ### CTPMon
    flags.DQ.Steering.doCTPMon = False

    if (partitionName != 'ATLAS' and partitionName != 'ATLAS_MP1'):
        flags.DQ.Steering.doHLTMon = False

    # ############################################################
    # Lock flags
    # ############################################################
    flags.lock()

    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    bytestreamConversion = CompFactory.ByteStreamCnvSvc()
    acc.addService(bytestreamConversion, primary=True)

    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(flags))
    
    bytestreamInput = acc.getService("ByteStreamInputSvc")

    # ############################################################
    # The name of the partition you want to connect
    # ############################################################
    bytestreamInput.Partition = partitionName

    bytestreamInput.Key = "dcm"
    bytestreamInput.KeyCount = 40

    # #######################################################
    # TimeOut (in milliseconds) - Prevent job with low rates 
    # to reconnect too often to SFIs
    # ######################################################
    bytestreamInput.Timeout = 600000    

    bytestreamInput.UpdatePeriod = 200

    bytestreamInput.ISServer = 'Histogramming'

    bytestreamInput.StreamNames = ['express']
    bytestreamInput.StreamType = "physics"
    bytestreamInput.StreamLogic = "Or"


    if partitionName == 'ATLAS' or partitionName == 'ATLAS_MP1':
        from PyUtils.OnlineISConfig import GetAtlasReady
        if beamType == BeamType.Cosmics:
            bytestreamInput.StreamNames = ['express','IDCosmic','HLT_IDCosmic','CosmicMuons','CosmicCalo']
        else:
            if GetAtlasReady():
                print ("ATLAS READY, reading express stream")
                bytestreamInput.StreamNames = ['express']
            else:
                print ("ATLAS NOT READY, reading standby stream")
                bytestreamInput.StreamNames = ['express','IDCosmic','HLT_IDCosmic','CosmicMuons','MinBias','Standby','Main','CosmicCalo']
    else:
        bytestreamInput.StreamLogic = 'Ignore'
        bytestreamInput.PublishName = 'GMT9_23_0_28_CA_test_1'

    # #######################################################################
    # Add BeamSpotDecoratorAlg to solve muon monitoring issue 
    # reported in https://its.cern.ch/jira/browse/ATLASRECTS-7281
    # This needs to be fixed by muon experts 
    # ######################################################################

    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoBeamSpotDecoratorAlgCfg
    acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))

    # ###########################################################
    # Load Reco
    # ###########################################################

    ### Calo
    if flags.Detector.EnableCalo:
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        acc.merge(CaloRecoCfg(flags))
    
    ### ID
    if flags.Reco.EnableTracking:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        acc.merge(InDetTrackRecoCfg(flags))
    
    ### Muon
    if flags.Detector.EnableMuon:
        from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
        acc.merge(MuonReconstructionCfg(flags))

    ### Muon Combined
    if flags.Reco.EnableCombinedMuon:
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import (
            MuonCombinedReconstructionCfg)
        acc.merge(MuonCombinedReconstructionCfg(flags))

    ### Jet
    if flags.Reco.EnableJet:
        from JetRecConfig.JetRecoSteering import JetRecoSteeringCfg
        acc.merge(JetRecoSteeringCfg(flags))

    ### Particle flow 
    if flags.Reco.EnablePFlow:
        from eflowRec.PFRun3Config import PFCfg
        acc.merge(PFCfg(flags))

    ### EGamma 
    if flags.Reco.EnableEgamma:
        from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
        acc.merge(EGammaSteeringCfg(flags))

    ### Tau
    if flags.Reco.EnableTau:
        from tauRec.TauConfig import TauReconstructionCfg
        acc.merge(TauReconstructionCfg(flags))

    ### Trigger
    if flags.Reco.EnableTrigger:    
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
        acc.merge(TriggerRecoCfg(flags))

    ### MET
    if flags.Reco.EnableMet:
        from METReconstruction.METRecCfg import METCfg
        acc.merge(METCfg(flags))

    ### Isolation
    if flags.Reco.EnableIsolation:
        from IsolationAlgs.IsolationSteeringConfig import IsolationSteeringCfg
        acc.merge(IsolationSteeringCfg(flags))

    ### Lucid
    if flags.Detector.EnableLucid:
        from ForwardRec.LucidRecConfig import LucidRecCfg
        acc.merge(LucidRecCfg(flags))

    ### Heavy Ion
    if flags.Reco.EnableHI:
        from HIRecConfig.HIRecConfig import HIRecCfg
        acc.merge(HIRecCfg(flags))

    ### Btagging
    if flags.Reco.EnableBTagging:
        from BTagging.BTagConfig import BTagRecoSplitCfg
        acc.merge(BTagRecoSplitCfg(flags))
 
    # #####################################################
    # Load Monitoring
    # #####################################################
    if flags.DQ.doMonitoring:
        from AthenaMonitoring.AthenaMonitoringCfg import (
            AthenaMonitoringCfg, AthenaMonitoringPostprocessingCfg)
        acc.merge(AthenaMonitoringCfg(flags))
        if flags.DQ.doPostProcessing:
            acc.merge(AthenaMonitoringPostprocessingCfg(flags))

         
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    acc.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2", db=""))

    # #######################################
    # Set TRT expert flags
    # #######################################
    acc.getEventAlgo("AlgTRTMonitoringRun3RAW").doExpert = True
    acc.getEventAlgo("AlgTRTMonitoringRun3RAW").doEfficiency = True

    # ###########################################################################################################################
    # Need to add this line since it was needed as explained in the link below. Otherwise jobs crash
    # https://gitlab.cern.ch/atlas/athena/-/blob/master/Reconstruction/RecExample/RecExOnline/share/RecExOnline_postconfig.py#L12
    # ########################################################################################################################### 
    acc.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/det/dqm/GlobalMonitoring/PoolFileCatalog_M7/PoolFileCatalog.xml"]

    # #######################################
    # Run 
    # #######################################
    sc = acc.run()
    sys.exit(0 if sc.isSuccess() else 1)
