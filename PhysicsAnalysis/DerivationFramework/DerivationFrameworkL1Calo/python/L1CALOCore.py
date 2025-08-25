#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# L1CALOCore.py
# Define the list of containers for the L1Calo derivations
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
from AthenaCommon.Constants import INFO
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, MetadataCategory

# 
def L1CALOCoreCfg(flags, deriv='L1CALO1', **kwargs):
    """
    Core fragment for L1CALO derivations
    """
    from AthenaCommon.Logging import logging
    log = logging.getLogger('L1CALO')
    log.info('Called L1CaloCore config for derivation %s',deriv)

    streamNameStem = "DAOD_" + deriv
    streamName =  "Stream" + streamNameStem 

    acc = ComponentAccumulator()

    # the derivation can also be run on pool files e.g. MC - need to switch off many decoders etc..
    # Note: static content not allowed when running on pool
    isNotPool = flags.Input.Format is not Format.POOL
    
    fillSuperCells=False

    # has the L1Calo simulation been configured by job flags
    isL1CaloSim = flags.Trigger.L1.doeFex and flags.Trigger.L1.dojFex and flags.Trigger.L1.dogFex

    # decode the legacy L1Calo information - required because flags.Trigger.doLVL1 is False
    if isNotPool:
        from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
        acc.merge(LVL1CaloRun2ReadBSCfg(flags))

    # set up thinning tools
    thinningTools = []

    # set up legacy trigger tower thinning for L1CALO1
    if 'L1CALO1' in deriv and isNotPool:
        from TrigT1CaloCalibTools.L1CaloCalibToolsConfig import LegacyTriggerTowerThinningCfg
        LegacyTowerThinningTool = acc.getPrimaryAndMerge(LegacyTriggerTowerThinningCfg(
            flags,
            name = "L1CALOCaloThinningTool",
            StreamName = streamName,
            TriggerTowerLocation = "xAODTriggerTowers",
            MinCaloCellET = 0.8,
            MinADC = 36,
            UseRandom = True,
            MinRandom = 0.01 ) )
        thinningTools.append(LegacyTowerThinningTool)

    # set up decorators
    # Legacy Trigger Tower decorator
    if isNotPool:
        from TrigT1CaloCalibTools.L1CaloCalibToolsConfig import LegacyTriggerTowerDecoratorCfg
        acc.merge(LegacyTriggerTowerDecoratorCfg(flags, name = 'L1CaloTriggerTowerDecorator'))
    
    # setup skimming tool example (AOD data only)
    skimmingTools = []
    if not isNotPool and not flags.Input.isMC:
        # example trigger skimming tool as with JETM10.py
        skimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "L1CALOSkimmingTool1",
                                                                           TriggerListOR = ["HLT_noalg_L1XE.*"] )
        acc.addPublicTool(skimmingTool, primary = True)
        skimmingTools.append(skimmingTool)

    #
    augmentationTools = []

    # Set up the derivation kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name = "DFL1CALO_KERN",
        AugmentationTools = augmentationTools,
        ThinningTools = thinningTools,
        SkimmingTools = skimmingTools,
        RunSkimmingFirst = not isNotPool,
        doChronoStat=(flags.Concurrency.NumThreads <= 1),
        OutputLevel = INFO))

    # Phase 1 setup
    # emulate/decorate the input towers
    # first need to decode LATOME readout
    if isNotPool:
        from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg
        acc.merge(ReadSCellFromByteStreamCfg(flags))

    # Emulate jFEX input towers
    emulatedDataTowersKey = "L1_jFexEmulatedTowers"
    if flags.Trigger.L1.dojFex and isNotPool:
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
        acc.merge(jFexEmulatedTowersCfg(flags, writeKey=emulatedDataTowersKey))

    # Decorate any jFEX data towers
    extraJfexInfo = False # for decorating with SCell inputs to the towers
    if flags.Trigger.L1.dojFex and isNotPool:
        from L1CaloFEXAlgos.L1CaloFEXAlgosConfig import L1CalojFEXDecoratorCfg
        acc.merge(L1CalojFEXDecoratorCfg(flags, ExtraInfo=extraJfexInfo))

    # Decorate the emulated jFEX towers
    if flags.Trigger.L1.dojFex and isNotPool:
        acc.merge(L1CalojFEXDecoratorCfg(flags,name='jFexTower2SCellEmulatedDecorator',jTowersReadKey=emulatedDataTowersKey,ExtraInfo=extraJfexInfo))

    # Emulate eFEX input towers
    if flags.Trigger.L1.doeFex and isNotPool:
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import eFexEmulatedTowersCfg
        eFexEmulatedTool = eFexEmulatedTowersCfg(flags,'L1_eFexEmulatedTowers')
        acc.merge(eFexEmulatedTool)


    # Re-simulate from LATOME (for both data and POOL files with SCells)
    from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
    if isNotPool:
        acc.merge(L1CaloFEXSimCfg(flags, simulateAltTau=True))
    elif isL1CaloSim:
        SCellType = flags.Trigger.L1.L1CaloSuperCellContainerName
        if SCellType in flags.Input.Collections:
            acc.merge(L1CaloFEXSimCfg(flags, simulateAltTau=True))
        # Need to deal with database for MC productions where L1Calo DB was not in global tag
        if flags.Input.isMC:
            from Campaigns.Utils import Campaign
            print("campaign",flags.Input.MCCampaign)
            if flags.Input.MCCampaign != Campaign.MC23e:
                if flags.Trigger.L1.doeFex:
                    from IOVDbSvc.IOVDbSvcConfig import addOverride
                    acc.merge(addOverride(flags, '/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts', 'EfexNoiseCuts-RUN3-MCDEFAULT-TEST-00'))
                    acc.merge(addOverride(flags, '/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib', 'EfexEnergyCalib-RUN3-MCDEFAULT-TEST-00'))
                if flags.Trigger.L1.dojFex:
                    from IOVDbSvc.IOVDbSvcConfig import addOverride
                    acc.merge(addOverride(flags, '/TRIGGER/L1Calo/V1/Calibration/JfexSystemSettings', 'JfexSystemSettings-RUN3-MCDEFAULT-TEST'))
                    acc.merge(addOverride(flags, '/TRIGGER/L1Calo/V1/Calibration/JfexModuleSettings', 'JfexModuleSettings-RUN3-MCDEFAULT-TEST'))
                    acc.merge(addOverride(flags, '/TRIGGER/L1Calo/V1/Calibration/JfexNoiseCuts', 'JfexNoiseCuts-RUN3-MCDEFAULT-TEST'))

    # decorate the eFEX TOBs 
    if flags.Trigger.L1.doeFex and isNotPool:
        # Temporary fix to ensure the L1 TOBs are decoded by the Trigger
        from TrigDecisionTool.TrigDecisionToolConfig import getRun3NavigationContainerFromInput
        ExtraInputs = [('xAOD::TrigCompositeContainer', 'StoreGateSvc+'+getRun3NavigationContainerFromInput(flags))]
        acc.addEventAlgo( CompFactory.LVL1.eFexTOBDecorator('eFexTOBDecorator',
                                                            eFexEMRoIContainer = 'L1_eEMRoI',
                                                            eFexTauRoIContainer = 'L1_eTauRoI',
                                                            ExtraInputs=ExtraInputs) )
        acc.addEventAlgo( CompFactory.LVL1.eFexTOBDecorator('eFexTOBDecoratorSim',
                                                            eFexEMRoIContainer = 'L1_eEMRoISim',
                                                            eFexTauRoIContainer = 'L1_eTauRoIAltSim',
                                                            ExtraInputs=[]) )
    
    if fillSuperCells:
        acc.addEventAlgo( CompFactory.LVL1.eFexTOBSuperCellDecorator('eFexTOBSuperCellDecoratorSim',
                                                                     eFexEMRoIContainer = 'L1_eEMRoISim',
                                                                     eFexTauRoIContainer = 'L1_eTauxRoISim') )

    # set up the slimming helper
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    L1CaloSlimmingHelper = SlimmingHelper("L1CaloSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    L1CaloSlimmingHelper.IncludeTriggerNavigation = True   # Trigger info is actually stored only when running on data...
    L1CaloSlimmingHelper.IncludeAdditionalTriggerContent = False # precludes EGamma/Jet TriggerContent 

    L1CaloSlimmingHelper.IncludeEGammaTriggerContent = True
    L1CaloSlimmingHelper.IncludeJetTriggerContent = True
    L1CaloSlimmingHelper.IncludeBJetTriggerContent = True
    L1CaloSlimmingHelper.IncludeTauTriggerContent = True # does not select any Run3 HLT collections - added below
    IncludeEtMissTriggerContent = True # very useful to have MET trigger info

    # Container selection based on share/L1CALO versions
    # Note: if the container is in the on-the-fly list (ContainersOnTheFly.py) then we do not have to add it to the dictionary
    # We can do smart slimming if the container is in the smart list (FullListOfSmartContainers.py)

    # if we do not run the simulation on MC, write SCells
    if flags.Input.isMC and not isL1CaloSim:
        L1CaloSlimmingHelper.AppendToDictionary.update({"SCell":"CaloCellContainer"})
        AllVariables += ["SCell"]

    # some gymnastics for HLT from RAWD
    if isNotPool and L1CaloSlimmingHelper.IncludeEGammaTriggerContent:
        # replicate adding EGammaTriggerContent
        # switch the eGamma Trigger off for RAW data - smart slimming not required
        L1CaloSlimmingHelper.IncludeEGammaTriggerContent = False
        ElToKeep = ['ptcone20', 'ptvarcone20', 'ptcone30', 'ptvarcone30', 'trk_d0','cl_eta2','cl_phi2', 'deltaEta1PearDistortion']
        ElVars = '.'.join(ElToKeep)
        StaticContent += ["xAOD::ElectronContainer#HLT_egamma_Electrons"]
        StaticContent += ["xAOD::ElectronAuxContainer#HLT_egamma_ElectronsAux."+ElVars]
        StaticContent += ["xAOD::ElectronContainer#HLT_egamma_Electrons_GSF"]
        StaticContent += ["xAOD::ElectronAuxContainer#HLT_egamma_Electrons_GSFAux."+ElVars]
        # non-slimmed
        L1CaloSlimmingHelper.AppendToDictionary.update({"HLT_CaloEMClusters_Electron":"xAOD::CaloClusterContainer",
                                                        "HLT_CaloEMClusters_ElectronAux":"xAOD::CaloClusterAuxContainer"})
        AllVariables += ["HLT_CaloEMClusters_Electron"]
        L1CaloSlimmingHelper.AppendToDictionary.update({"HLT_IDTrack_Electron_IDTrig":"xAOD::TrackParticleContainer",
                                                        "HLT_IDTrack_Electron_IDTrigAux":"xAOD::TrackParticleAuxContainer"})
        AllVariables += ["HLT_IDTrack_Electron_IDTrig"]
        L1CaloSlimmingHelper.AppendToDictionary.update({"HLT_IDTrack_Electron_GSF":"xAOD::TrackParticleContainer",
                                                        "HLT_IDTrack_Electron_GSFAux":"xAOD::TrackParticleAuxContainer"})
        AllVariables += ["HLT_IDTrack_Electron_GSF"]

    if isNotPool and L1CaloSlimmingHelper.IncludeJetTriggerContent:
        # replicate adding JetTriggerContent - easier because this is genuine smart slimming
        # Update the dictionary with the containers that will be smart slimmed
        L1CaloSlimmingHelper.AppendToDictionary.update ({"HLT_AntiKt4EMTopoJets_nojcalib":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMTopoJets_nojcalibAux":"xAOD::JetAuxContainer",
                                                         "HLT_AntiKt4EMTopoJets_nojcalib_ftf":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMTopoJets_nojcalib_ftfAux":"xAOD::JetAuxContainer",
                                                         "HLT_AntiKt4EMTopoJets_subjesIS":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMTopoJets_subjesISAux":"xAOD::ShallowAuxContainer",
                                                         "HLT_AntiKt4EMPFlowJets_nojcalib_ftf":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMPFlowJets_nojcalib_ftfAux":"xAOD::JetAuxContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftfAux":"xAOD::ShallowAuxContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subjesgscIS_ftf":"xAOD::JetContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subjesgscIS_ftfAux":"xAOD::ShallowAuxContainer",
                                                         "HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf":"xAOD::JetContainer",
                                                         "HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftfAux":"xAOD::JetAuxContainer",
                                                         "HLT_IDVertex_FS":"xAOD::VertexContainer",
                                                         "HLT_IDVertex_FSAux":"xAOD::VertexAuxContainer",
                                                         "HLT_TCEventInfo_jet":"xAOD::TrigCompositeContainer",
                                                         "HLT_TCEventInfo_jetAux":"xAOD::TrigCompositeAuxContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging": "xAOD::BTaggingContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingAux": "xAOD::BTaggingAuxContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets": "xAOD::JetContainer",
                                                         "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux": "xAOD::JetAuxContainer"})

    AllVariables += ["HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging", "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets"]

    if L1CaloSlimmingHelper.IncludeTauTriggerContent:
        L1CaloSlimmingHelper.AppendToDictionary.update ({"HLT_TrigTauRecMerged_MVA":"xAOD::TauJetContainer",
                                                         "HLT_TrigTauRecMerged_MVAAux":"xAOD::TauJetAuxContainer",
                                                         "HLT_TrigTauRecMerged_LLP":"xAOD::TauJetContainer",
                                                         "HLT_TrigTauRecMerged_LLPAux":"xAOD::TauJetAuxContainer",
                                                         "HLT_TrigTauRecMerged_LRT":"xAOD::TauJetContainer",
                                                         "HLT_TrigTauRecMerged_LRTAux":"xAOD::TauJetAuxContainer",
                                                         "HLT_TrigTauRecMerged_CaloMVAOnly":"xAOD::TauJetContainer",
                                                         "HLT_TrigTauRecMerged_CaloMVAOnlyAux":"xAOD::TauJetAuxContainer"})
        AllVariables += ["HLT_TrigTauRecMerged_MVA", "HLT_TrigTauRecMerged_LLP", "HLT_TrigTauRecMerged_LRT", "HLT_TrigTauRecMerged_CaloMVAOnly"]

    if IncludeEtMissTriggerContent:
        L1CaloSlimmingHelper.AppendToDictionary.update ({"HLT_MET_tcpufit":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_tcpufitAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_cell":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_cellAux":"xAOD::TrigMissingETAuxContainer", # not present
                                                         "HLT_MET_trkmht":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_trkmhtAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_cvfpufit":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_cvfpufitAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_pfopufit":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_pfopufitAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_mhtpufit_em":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_mhtpufit_emAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_mhtpufit_pf":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_mhtpufit_pfAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_pfsum":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_pfsumAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_pfsum_vssk":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_pfsum_vsskAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_pfsum_cssk":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_pfsum_csskAux":"xAOD::TrigMissingETAuxContainer",
                                                         "HLT_MET_nn":"xAOD::TrigMissingETContainer",
                                                         "HLT_MET_nnAux":"xAOD::TrigMissingETAuxContainer"})

        AllVariables += ["HLT_MET_tcpufit","HLT_MET_cell","HLT_MET_trkmht","HLT_MET_cvfpufit","HLT_MET_pfopufit","HLT_MET_mhtpufit_em","HLT_MET_mhtpufit_pf","HLT_MET_pfsum","HLT_MET_pfsum_vssk","HLT_MET_pfsum_cssk","HLT_MET_nn"]

    # Generic event info
    L1CaloSlimmingHelper.AppendToDictionary.update({"EventInfo":"xAOD::EventInfo","EventInfoAux":"xAOD::EventAuxInfo"}) 

    # We keep all of EventInfo rather than smart slim
    AllVariables += ["EventInfo","Kt4EMPFlowEventShape"]

    # Physics Objects 
    # Those for which there is smart slimming but not in the on the fly list
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"Muons":"xAOD::MuonContainer", "MuonsAux":"xAOD::MuonAuxContainer",
         "Photons":"xAOD::PhotonContainer", "PhotonsAux":"xAOD::PhotonAuxContainer",
         "TauJets":"xAOD::TauJetContainer", "TauJetsAux":"xAOD::TauJetAuxContainer"}
    )
    AllVariables += ["AntiKt4EMPFlowJets","AntiKt10LCTopoJets","Muons","Photons"]
    AllVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"]
    # TauJets require smart slimming in order not to cause issues
    SmartCollections += ["TauJets"]

    # Use unslimmed electrons in order to use Likelihood qualities
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"Electrons":"xAOD::ElectronContainer", "ElectronsAux":"xAOD::ElectronAuxContainer"} )
    AllVariables += ["Electrons"]

    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"ForwardElectrons":"xAOD::ElectronContainer", "ForwardElectronsAux":"xAOD::ElectronAuxContainer"} )
    AllVariables += ["ForwardElectrons"]

    # Missing ET - unslimmed container
    AllVariables += ["METAssoc_AntiKt4EMPFlow"]
        
    # using MET slimming as per share/L1CALOX.py
    if isNotPool:
        StaticContent += ["xAOD::MissingETContainer#MET_Reference_AntiKt4EMPFlow"]
        StaticContent += ["xAOD::MissingETAuxContainer#MET_Reference_AntiKt4EMPFlowAux.-ConstitObjectLinks.-ConstitObjectWeights"]

        StaticContent += ["xAOD::MissingETContainer#MET_Core_AntiKt4EMPFlow"]
        StaticContent += ["xAOD::MissingETAuxContainer#MET_Core_AntiKt4EMPFlowAux.name.mpx.mpy.sumet.source"]

        StaticContent += ["xAOD::MissingETContainer#MET_Track"]
        StaticContent += ["xAOD::MissingETAuxContainer#MET_TrackAux.name.mpx.mpy"]
    else:
        AllVariables += ["MET_Reference_AntiKt4EMPFlow"]
        AllVariables += ["MET_Core_AntiKt4EMPFlow"]       
        L1CaloSlimmingHelper.AppendToDictionary.update (
            {"MET_Track":"xAOD::MissingETContainer", "MET_TrackAux":"xAOD::MissingETAuxContainer"} )
        AllVariables += ["MET_Track"]

    # Primary vertices
    if isNotPool:
        StaticContent += ["xAOD::VertexContainer#PrimaryVertices"]
        StaticContent += ["xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"]
    else:
        L1CaloSlimmingHelper.AppendToDictionary.update(
            {"PrimaryVertices":"xAOD::VertexContainer","PrimaryVerticesAux":"xAOD::VertexAuxContainer"}
        )
        AllVariables += ["PrimaryVertices"]


    # Egamma CP additions
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"egammaTopoSeededClusters":"xAOD::CaloClusterContainer","egammaTopoSeededClustersAux":"xAOD::CaloClusterAuxContainer"}
    )
    AllVariables += ["egammaTopoSeededClusters"]
    
    # GSF vertices and tracks
    if isNotPool:
        StaticContent += ["xAOD::VertexContainer#GSFConversionVertices"]
        # we have to disable vxTrackAtVertex branch since it is not xAOD compatible
        StaticContent += ["xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex"]
        #
        trackParticleAuxExclusions="-caloExtension.-cellAssociation.-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
        StaticContent += ["xAOD::TrackParticleContainer#GSFTrackParticles"]
        StaticContent += ["xAOD::TrackParticleAuxContainer#GSFTrackParticlesAux."+trackParticleAuxExclusions]
    else:
        L1CaloSlimmingHelper.AppendToDictionary.update (
            {"GSFConversionVertices":"xAOD::VertexContainer",
             "GSFConversionVerticesAux":"xAOD::VertexAuxContainer",
             "GSFTrackParticles":"xAOD::TrackParticleContainer",
             "GSFTrackParticlesAux":"xAOD::TrackParticleAuxContainer"}
        )
        AllVariables += ["GSFConversionVertices","GSFTrackParticles"]

    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"egammaClusters":"xAOD::CaloClusterContainer",
         "egammaClustersAux":"xAOD::CaloClusterAuxContainer",
         "TauPi0Clusters":"xAOD::CaloClusterContainer",
         "TauPi0ClustersAux":"xAOD::CaloClusterAuxContainer",
         "CaloCalTopoClusters":"xAOD::CaloClusterContainer",
         "CaloCalTopoClustersAux":"xAOD::CaloClusterAuxContainer",
         "MuonSegments":"xAOD::MuonSegmentContainer",
         "MuonSegmentsAux":"xAOD::MuonSegmentAuxContainer"}
    )
    AllVariables += ["egammaClusters","TauPi0Clusters","CaloCalTopoClusters","MuonSegments"]

    # Add b-tagging information directly using ExtraVariables - as done for PHYSLITE
    ExtraVariables += [
        'BTagging_AntiKt4EMPFlow.DL1dv01_pu.DL1dv01_pc.DL1dv01_pb.GN2v00_pu.GN2v00_pc.GN2v00_pb'
    ]

    # L1Calo information
    # Legacy RoI Containers
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"LVL1EmTauRoIs":"xAOD::EmTauRoIContainer",
         "LVL1EmTauRoIsAux":"xAOD::EmTauRoIAuxContainer",
         "LVL1EnergySumRoI":"xAOD::EnergySumRoI",
         "LVL1EnergySumRoIAux":"xAOD::EnergySumRoIAuxInfo",
         "LVL1JetEtRoI":"xAOD::JetEtRoI",
         "LVL1JetEtRoIAux":"xAOD::JetEtRoIAuxInfo",
         "LVL1JetRoIs":"xAOD::JetRoIContainer",
         "LVL1JetRoIsAux":"xAOD::JetRoIAuxContainer",
         "LVL1MuonRoIs":"xAOD::MuonRoIContainer",
         "LVL1MuonRoIsAux":"xAOD::MuonRoIAuxContainer"}
    )
    AllVariables += [ "LVL1EmTauRoIs","LVL1EnergySumRoI","LVL1JetEtRoI","LVL1JetRoIs","LVL1MuonRoIs"]

    # Legacy sub-systems
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"JEMTobRoIsRoIB":"xAOD::JEMTobRoIContainer",
         "JEMTobRoIsRoIBAux":"xAOD::JEMTobRoIAuxContainer",
         "JEMTobRoIs":"xAOD::JEMTobRoIContainer",
         "JEMTobRoIsAux":"xAOD::JEMTobRoIAuxContainer",
         "JEMEtSums":"xAOD::JEMEtSumsContainer",
         "JEMEtSumsAux":"xAOD::JEMEtSumsAuxContainer"}
    )
    AllVariables += ["JEMTobRoIsRoIB",
                     "JEMTobRoIs",
                     "JEMEtSums"]
    
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"CMXCPHits":"xAOD::CMXCPHitsContainer",
         "CMXCPHitsAux":"xAOD::CMXCPHitsAuxContainer",
         "CMXCPTobs":"xAOD::CMXCPTobContainer",
         "CMXCPTobsAux":"xAOD::CMXCPTobAuxContainer",
         "CMXEtSums":"xAOD::CMXEtSumsContainer",
         "CMXEtSumsAux":"xAOD::CMXEtSumsAuxContainer",
         "CMXJetHits":"xAOD::CMXJetHitsContainer",
         "CMXJetHitsAux":"xAOD::CMXJetHitsAuxContainer",
         "CMXJetTobs":"xAOD::CMXJetTobContainer",
         "CMXJetTobsAux":"xAOD::CMXJetTobAuxContainer",
         "CMXRoIs":"xAOD::CMXRoIContainer",
         "CMXRoIsAux":"xAOD::CMXRoIAuxContainer"}
    )
    AllVariables += ["CMXCPHits",
                     "CMXCPTobs",
                     "CMXEtSums",
                     "CMXJetHits",
                     "CMXJetTobs",
                     "CMXRoIs"]


    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"CPMTobRoIs":"xAOD::CPMTobRoIContainer",
         "CPMTobRoIsAux":"xAOD::CPMTobRoIAuxContainer",
         "CPMTobRoIsRoIB":"xAOD::CPMTobRoIContainer",
         "CPMTobRoIsRoIBAux":"xAOD::CPMTobRoIAuxContainer",
         "CPMTowers":"xAOD::CPMTowerContainer",
         "CPMTowersAux":"xAOD::CPMTowerAuxContainer",
         "CPMTowersOverlap":"xAOD::CPMTowerContainer",
         "CPMTowersOverlapAux":"xAOD::CPMTowerAuxContainer",
         "RODHeaders":"xAOD::RODHeaderContainer",
         "RODHeadersAux":"xAOD::RODHeaderAuxContainer",
         "xAODTriggerTowers":"xAOD::TriggerTowerContainer",
         "xAODTriggerTowersAux":"xAOD::TriggerTowerAuxContainer",
         "JetElements":"xAOD::JetElementContainer",
         "JetElementsAux":"xAOD::JetElementAuxContainer",
         "JetElementsOverlap":"xAOD::JetElementContainer",
         "JetElementsOverlapAux":"xAOD::JetElementAuxContainer",
         "L1TopoRawData":"xAOD::L1TopoRawDataContainer",
         "L1TopoRawDataAux":"xAOD::L1TopoRawDataAuxContainer",
         "L1_Phase1L1TopoRAWData":"xAOD::L1TopoRawDataContainer",
         "L1_Phase1L1TopoRAWDataAux":"xAOD::L1TopoRawDataAuxContainer"}
    )
    AllVariables += ["CPMTobRoIs",
                     "CPMTobRoIsRoIB",
                     "CPMTowers",
                     "CPMTowersOverlap",
                     "RODHeaders",
                     "xAODTriggerTowers",
                     "JetElements",
                     "JetElementsOverlap",
                     "L1TopoRawData",
                     "L1_Phase1L1TopoRAWData"]

    
    # Phase 1
    # TOBs from HLT
    L1CaloSlimmingHelper,AllVariables = addEfexTOBs(L1CaloSlimmingHelper, AllVariables)
    L1CaloSlimmingHelper,AllVariables = addJfexTOBs(L1CaloSlimmingHelper, AllVariables)
    L1CaloSlimmingHelper,AllVariables = addGfexTOBs(L1CaloSlimmingHelper, AllVariables)

    # re-simulated 
    L1CaloSlimmingHelper,AllVariables = addEfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")
    L1CaloSlimmingHelper,AllVariables = addJfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")
    L1CaloSlimmingHelper,AllVariables = addGfexTOBs(L1CaloSlimmingHelper, AllVariables, "Sim")


    # FEX input data towers
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_eFexDataTowers":"xAOD::eFexTowerContainer",
         "L1_eFexDataTowersAux":"xAOD::eFexTowerAuxContainer",
         "L1_jFexDataTowers":"xAOD::jFexTowerContainer",
         "L1_jFexDataTowersAux":"xAOD::jFexTowerAuxContainer",
         "L1_gFexDataTowers":"xAOD::gFexTowerContainer",
         "L1_gFexDataTowersAux":"xAOD::gFexTowerAuxContainer",
         "L1_gFexDataTowers50":"xAOD::gFexTowerContainer",
         "L1_gFexDataTowers50Aux":"xAOD::gFexTowerAuxContainer"}
    )
    AllVariables += ["L1_eFexDataTowers","L1_jFexDataTowers","L1_gFexDataTowers","L1_gFexDataTowers50"]

    # Emulated eFEX input tower data from LATOME
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_eFexEmulatedTowers":"xAOD::eFexTowerContainer",
         "L1_eFexEmulatedTowersAux":"xAOD::eFexTowerAuxContainer"}
    )
    AllVariables += ["L1_eFexEmulatedTowers"]
    
    # Emulated jFEX input tower data from LATOME    
    L1CaloSlimmingHelper.AppendToDictionary.update (
        {"L1_jFexEmulatedTowers":"xAOD::jFexTowerContainer",
         "L1_jFexEmulatedTowersAux":"xAOD::jFexTowerAuxContainer"}
    )    
    AllVariables += ["L1_jFexEmulatedTowers"]

    # For MC, add emulated gFEX input towers
    if flags.Input.isMC:
        L1CaloSlimmingHelper.AppendToDictionary.update (
            {"L1_gFexEmulatedTowers":"xAOD::gFexTowerContainer",
             "L1_gFexEmulatedTowersAux":"xAOD::gFexTowerAuxContainer"}
        )
        AllVariables += [ "L1_gFexEmulatedTowers" ]

    # In case MC has no jets, b-tagging or MET, schedule reconstruction
    if flags.Input.isMC:
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
        from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_noVR, AntiKt10UFOCSSKSoftDrop_trigger
        jets_to_schedule = [jet for jet in (AntiKt4EMPFlow, AntiKt10LCTopo_noVR, AntiKt10UFOCSSKSoftDrop_trigger)
                             if jet.fullname() not in flags.Input.Collections]

        if jets_to_schedule:
            from JetRecConfig.JetRecConfig import JetRecCfg
            for container in jets_to_schedule:
                acc.merge(JetRecCfg(flags, container))

        jet_collections = set([_.fullname().replace('Jets','') for _ in jets_to_schedule])
        btag_jet_collections = set(['AntiKt4EMPFlow'])
        met_jet_collections = set(['AntiKt4EMPFlow'])

        if jet_collections & btag_jet_collections:
            log.info('Scheduling b-tagging of rebuilt jets')
            from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
            acc.merge(BeamSpotCondAlgCfg(flags))
            from BTagging.BTagConfig import BTagRecoSplitCfg
            # 
            for container in jet_collections & btag_jet_collections:
                acc.merge(BTagRecoSplitCfg(flags, [container]))

        # MET
        if jet_collections & met_jet_collections:
             log.info('Scheduling rebuild of standard MET')
             from METReconstruction.METAssociatorCfg import METAssociatorCfg
             from METUtilities.METMakerConfig import getMETMakerAlg
             for container in jet_collections & met_jet_collections:
                 if container == 'AntiKt4EMPFlow':
                     # build links between FlowElements and electrons, photons, muons and taus
                     log.info('Scheduling FlowElement linking')
                     from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
                     acc.merge(PFGlobalFlowElementLinkingCfg(flags))
                 acc.merge(METAssociatorCfg(flags, container))
                 acc.addEventAlgo(getMETMakerAlg(container))
             from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
             acc.merge(CaloNoiseCondAlgCfg(flags)) # Prereq for Calo MET
             from METReconstruction.METCalo_Cfg import METCalo_Cfg
             acc.merge(METCalo_Cfg(flags))


    # Truth collections
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import PreJetMCTruthAugmentationsCfg
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTruthCollectionNavigationDecorationsCfg,AddBornLeptonCollectionCfg
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddHardScatterCollectionCfg,PostJetMCTruthAugmentationsCfg

        acc.merge(PreJetMCTruthAugmentationsCfg(flags,decorationDressing = 'dressedPhoton'))
        #
        from JetRecConfig.StandardSmallRJets import AntiKt4Truth,AntiKt4TruthWZ,AntiKt4TruthDressedWZ,AntiKtVRTruthCharged
        from JetRecConfig.StandardLargeRJets import AntiKt10TruthTrimmed,AntiKt10TruthSoftDrop
        from JetRecConfig.JetRecConfig import JetRecCfg
        jetList = [AntiKt4Truth,AntiKt4TruthWZ,AntiKt4TruthDressedWZ,AntiKtVRTruthCharged,
                   AntiKt10TruthTrimmed,AntiKt10TruthSoftDrop]

        for jd in jetList:
            if jd.fullname() not in flags.Input.Collections:
                acc.merge(JetRecCfg(flags,jd))

        if ( "xAOD::MissingETContainer#MET_Truth") not in flags.Input.TypedCollections:
            from METReconstruction.METTruth_Cfg import METTruth_Cfg
            acc.merge(METTruth_Cfg(flags))

        acc.merge(PostJetMCTruthAugmentationsCfg(flags, decorationDressing = 'dressedPhoton'))
        acc.addEventAlgo(CompFactory.DerivationFramework.LockDecorations(name ="L1CALOTruthContentsLockDecoration", Decorations = ['TruthParticles.dressedPhoton']))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(flags, ["TruthElectrons", "TruthMuons", "TruthPhotons", "TruthTaus", "TruthNeutrinos"]))
        # Special collection for Born leptons
        acc.merge(AddBornLeptonCollectionCfg(flags))
        # Special collection for hard scatter (matrix element) - save TWO extra generations of particles
        acc.merge(AddHardScatterCollectionCfg(flags, 2))

        L1CaloSlimmingHelper.AppendToDictionary.update (
            {
                'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer'}
        )

        AllVariables += [
            "TruthElectrons",
            "TruthMuons",
            "TruthPhotons",
            "TruthTaus",
            "TruthNeutrinos",
            "BornLeptons",
            "HardScatterParticles",
            "MET_Truth",
            "AntiKt4TruthJets",
            "AntiKt4TruthWZJets",
            "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
            "AntiKt10TruthSoftDropBeta100Zcut10Jets"
        ]

    L1CaloSlimmingHelper.AllVariables = AllVariables
    L1CaloSlimmingHelper.StaticContent = StaticContent
    L1CaloSlimmingHelper.SmartCollections = SmartCollections
    L1CaloSlimmingHelper.ExtraVariables = ExtraVariables
    
    # Output stream    
    L1CaloItemList = L1CaloSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, streamNameStem, ItemList=L1CaloItemList, AcceptAlgs=["DFL1CALO_KERN"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, streamNameStem, AcceptAlgs=["DFL1CALO_KERN"], createMetadata=[MetadataCategory.CutFlowMetaData,MetadataCategory.TriggerMenuMetaData]))

    return acc


def addEfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of eFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_eEMRoI"+postFix : "xAOD::eFexEMRoIContainer",
         "L1_eEMRoI"+postFix+"Aux" : "xAOD::eFexEMRoIAuxContainer",
         "L1_eTauRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_eTauRoIAlt"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauRoIAlt"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_cTauRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_cTauRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_eEMxRoI"+postFix : "xAOD::eFexEMRoIContainer",
         "L1_eEMxRoI"+postFix+"Aux" : "xAOD::eFexEMRoIAuxContainer",
         "L1_eTauxRoI"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauxRoI"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer",
         "L1_eTauxRoIAlt"+postFix : "xAOD::eFexTauRoIContainer",
         "L1_eTauxRoIAlt"+postFix+"Aux" : "xAOD::eFexTauRoIAuxContainer"} )    

    allVariables += ["L1_eEMRoI" + postFix,
                     "L1_eTauRoI" + postFix,
                     "L1_eTauRoIAlt" + postFix,
                     "L1_cTauRoI" + postFix,
                     "L1_eEMxRoI" + postFix,
                     "L1_eTauxRoI" + postFix,
                     "L1_eTauxRoIAlt" + postFix]
    
    return slimminghelper, allVariables

        
def addJfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of jFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_jFexMETRoI"+postFix : "xAOD::jFexMETRoIContainer",
         "L1_jFexMETRoI"+postFix+"Aux" : "xAOD::jFexMETRoIAuxContainer",
         "L1_jFexTauRoI"+postFix : "xAOD::jFexTauRoIContainer",
         "L1_jFexTauRoI"+postFix+"Aux" : "xAOD::jFexTauRoIAuxContainer",
         "L1_jFexFwdElRoI"+postFix : "xAOD::jFexFwdElRoIContainer",
         "L1_jFexFwdElRoI"+postFix+"Aux" : "xAOD::jFexFwdElRoIAuxContainer",
         "L1_jFexSRJetRoI"+postFix : "xAOD::jFexSRJetRoIContainer",
         "L1_jFexSRJetRoI"+postFix+"Aux" : "xAOD::jFexSRJetRoIAuxContainer",
         "L1_jFexLRJetRoI"+postFix : "xAOD::jFexLRJetRoIContainer",
         "L1_jFexLRJetRoI"+postFix+"Aux" : "xAOD::jFexLRJetRoIAuxContainer",
         "L1_jFexSumETRoI"+postFix : "xAOD::jFexSumETRoIContainer",
         "L1_jFexSumETRoI"+postFix+"Aux" : "xAOD::jFexSumETRoIAuxContainer",
         "L1_jFexMETxRoI"+postFix : "xAOD::jFexMETRoIContainer",
         "L1_jFexMETxRoI"+postFix+"Aux" : "xAOD::jFexMETRoIAuxContainer",
         "L1_jFexTauxRoI"+postFix : "xAOD::jFexTauRoIContainer",
         "L1_jFexTauxRoI"+postFix+"Aux" : "xAOD::jFexTauRoIAuxContainer",
         "L1_jFexFwdElxRoI"+postFix : "xAOD::jFexFwdElRoIContainer",
         "L1_jFexFwdElxRoI"+postFix+"Aux" : "xAOD::jFexFwdElRoIAuxContainer",
         "L1_jFexSRJetxRoI"+postFix : "xAOD::jFexSRJetRoIContainer",
         "L1_jFexSRJetxRoI"+postFix+"Aux" : "xAOD::jFexSRJetRoIAuxContainer",
         "L1_jFexLRJetxRoI"+postFix : "xAOD::jFexLRJetRoIContainer",
         "L1_jFexLRJetxRoI"+postFix+"Aux" : "xAOD::jFexLRJetRoIAuxContainer",
         "L1_jFexSumETxRoI"+postFix : "xAOD::jFexSumETRoIContainer",
         "L1_jFexSumETxRoI"+postFix+"Aux" : "xAOD::jFexSumETRoIAuxContainer"})

    allVariables += ["L1_jFexMETRoI" + postFix,
                     "L1_jFexTauRoI" + postFix,
                     "L1_jFexFwdElRoI" + postFix,
                     "L1_jFexSRJetRoI" + postFix,
                     "L1_jFexLRJetRoI" + postFix,
                     "L1_jFexSumETRoI" + postFix,
                     "L1_jFexMETxRoI" + postFix,
                     "L1_jFexTauxRoI" + postFix,
                     "L1_jFexFwdElxRoI" + postFix,
                     "L1_jFexSRJetxRoI" + postFix,
                     "L1_jFexLRJetxRoI" + postFix,
                     "L1_jFexSumETxRoI" + postFix]

    return slimminghelper, allVariables


def addGfexTOBs(slimminghelper, allVariables, postFix = ""):
    """
    add the list of gFEX containers for given postFix string
    """
    slimminghelper.AppendToDictionary.update (
        {"L1_gFexRhoRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexRhoRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gFexSRJetRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexSRJetRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gScalarEJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gScalarEJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gFexLRJetRoI"+postFix : "xAOD::gFexJetRoIContainer",
         "L1_gFexLRJetRoI"+postFix+"Aux" : "xAOD::gFexJetRoIAuxContainer",
         "L1_gMETComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMETComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gMHTComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMHTComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer",
         "L1_gMSTComponentsJwoj"+postFix : "xAOD::gFexGlobalRoIContainer",
         "L1_gMSTComponentsJwoj"+postFix+"Aux" : "xAOD::gFexGlobalRoIAuxContainer"})

    allVariables += ["L1_gFexRhoRoI" + postFix,
                     "L1_gFexSRJetRoI" + postFix,
                     "L1_gScalarEJwoj" + postFix,
                     "L1_gFexLRJetRoI" + postFix,
                     "L1_gMETComponentsJwoj" + postFix,
                     "L1_gMHTComponentsJwoj" + postFix,
                     "L1_gMSTComponentsJwoj" + postFix]

    return slimminghelper, allVariables
