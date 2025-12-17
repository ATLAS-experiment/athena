# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# TRIG8.py
# This defines DAOD_TRIG8, a DAOD format for Run 3.
# It contains the variables and objects needed ID Trigger performance
# such as online and offline tracks, RoIs, and offline objects.  
# Only events passing idperf, and similar, chains are kept.
# It requires the flag TRIG8 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


# Main algorithm config
def TRIG8KernelCfg(flags, name='TRIG8Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TRIG8"""
    acc = ComponentAccumulator()

    # Augmentations

    TRIG8MergedElectronContainer = "Electrons"
    TRIG8MergedMuonContainer = "Muons"
    if flags.Tracking.doLargeD0:
        # LRT track merge
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))

        # LRT muons merge
        TRIG8MergedMuonContainer = "StdWithLRTMuons"
        from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
        acc.merge(LRTMuonMergerAlg( flags,
                                    PromptMuonLocation    = "Muons",
                                    LRTMuonLocation       = "MuonsLRT",
                                    OutputMuonLocation    = TRIG8MergedMuonContainer,
                                    CreateViewCollection  = True))

        # LRT electrons merge
        TRIG8MergedElectronContainer = "StdWithLRTElectrons"
        from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
        acc.merge(LRTElectronMergerAlg( flags,
                                        PromptElectronLocation = "Electrons",
                                        LRTElectronLocation    = "LRTElectrons",
                                        OutputCollectionName   = TRIG8MergedElectronContainer,
                                        isDAOD                 = False,
                                        CreateViewCollection   = True))


    augmentationTools = [ ]

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    if flags.Tracking.doLargeD0:
        # LRT Egamma
        from DerivationFrameworkEGamma.EGammaLRTConfig import EGammaLRTCfg
        acc.merge(EGammaLRTCfg(flags))

        from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronLHSelectorsCfg
        acc.merge(LRTElectronLHSelectorsCfg(flags))

        # LRT Muons
        from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
        acc.merge(MuonsCommonCfg(flags, suff="LRT"))
    
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod

    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(flags)
    trig_all = list(TriggerAPI.getAllHLT(allperiods).keys())
    
    # Add in Run 3 triggers
    TriggerListsHelper = kwargs['TriggerListsHelper']
    trig_all += TriggerListsHelper.Run3TriggerNames
    
    #get all displaced jet triggers and all bjet triggers
    displaced_jet_triggers = [t for t in trig_all if "dispjet" in t]
    bjet_veto = ["HLT_e26_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1EM22VHI"]
    bjet_triggers = [t for t in trig_all if "boffperf" in t and t not in bjet_veto]

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
    from DerivationFrameworkTrigger.TriggerGenericObjectThinningConfig import TriggerGenericThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations

    TRIG8PhotonsThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(
        flags,
        name            = "TRIG8PhotonsThinningTool",
        StreamName      = kwargs['StreamName'],
        ContainerName   = "Photons",
        SelectionString = "Photons.pt >= 1000000."))

    TRIG8TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "TRIG8TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "InDetTrackParticles.pt > 1*GeV",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    if flags.Tracking.doLargeD0:
        TRIG8LRTTrackParticleThinningTool = acc.getPrimaryAndMerge(
            TrackParticleThinningCfg(
                flags, name = "TRIG8LRTTrackParticleThinningTool",
                StreamName              = kwargs['StreamName'],
                SelectionString         = "InDetLargeD0TrackParticles.pt > 1*GeV",
                InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))

    # Finally the kernel itself
    thinningTools = [TRIG8PhotonsThinningTool,
                     TRIG8TrackParticleThinningTool]
    if flags.Tracking.doLargeD0:
        thinningTools += [TRIG8LRTTrackParticleThinningTool]
    
    if((not flags.Input.isMC) or "HLT_AntiKt4EMTopoJets_subjesIS" in flags.Input.Collections):
        TRIG8JETThinningTool = acc.getPrimaryAndMerge(TriggerGenericThinningCfg(
            flags,
            name = "TRIG8JetThinningTool",
            StreamName = kwargs['StreamName'],
            ContainerName   = "HLT_AntiKt4EMTopoJets_subjesIS",
            TriggerListOR = sorted(list(set(displaced_jet_triggers + bjet_triggers)))
        ))

        thinningTools.append(TRIG8JETThinningTool)

    # Skimming
    skimmingTools = []

    if flags.Trigger.EDMVersion >= 0:
        # Pieces of trigger names to keep
        idtrig_keys = ['idperf', 'boffperf', 'ivarperf', 'idtp']
        # Triggers to veto
        idtrig_veto = ['HLT_e26_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1EM22VHI']
        # Add specific triggers
        additional_triggers = [
            "HLT_mu20_msonly",
            "HLT_j45_pf_ftf_preselj20_L1J15",
            "HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1XE55",
            "HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1XE55",
            "HLT_xe80_tcpufit_dedxtrk50_medium_L1XE50",
            "HLT_xe80_tcpufit_distrk20_medium_L1XE50",
            "HLT_xe80_tcpufit_distrk20_tight_L1XE50",
            "HLT_mu60_L1MU14FCH"
        ]
        idtrig_keys += additional_triggers
        idtrig_keys += displaced_jet_triggers

        triggers = [t for t in trig_all for k in idtrig_keys if k in t]
        for veto in idtrig_veto:
            try:
                triggers.remove(veto)
            except ValueError:
                print(f"Warning, {veto} already removed from trigger list.")

        #remove duplicates
        triggers = sorted(list(set(triggers)))
        print('TRIG8 list of triggers used for skimming:')
        for trig in triggers: print(trig)

        TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
        TRIG8TriggerSkimmingTool = TriggerSkimmingTool(name = "TRIG8TriggerPreSkimmingTool",
                                                       TriggerListAND = [],
                                                       TriggerListOR  = triggers)
        acc.addPublicTool(TRIG8TriggerSkimmingTool)

        skimmingTools.append(TRIG8TriggerSkimmingTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = skimmingTools,
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))

    return acc


def TRIG8Cfg(flags):

    acc = ComponentAccumulator()

    TRIG8MergedElectronContainer = (
        "StdWithLRTElectrons" if flags.Tracking.doLargeD0 else "Electrons")
    TRIG8MergedMuonContainer = (
        "StdWithLRTMuons" if flags.Tracking.doLargeD0 else "Muons")

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TRIG8TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(TRIG8KernelCfg(flags, name="TRIG8Kernel", StreamName = 'StreamDAOD_TRIG8', TriggerListsHelper = TRIG8TriggerListsHelper))


    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    TRIG8SlimmingHelper = SlimmingHelper("TRIG8SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    TRIG8SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "TauJets"
                                            ]
    if flags.Tracking.doLargeD0:
        TRIG8SlimmingHelper.SmartCollections += ["LRTElectrons", "MuonsLRT",
                                                 "InDetLargeD0TrackParticles"]

    TRIG8SlimmingHelper.AllVariables = ["HLT_IDTrack_Electron_FTF", 
                                        "HLT_IDTrack_ElecLRT_FTF", 
                                        "HLT_IDTrack_Electron_IDTrig", 
                                        "HLT_IDTrack_ElecLRT_IDTrig", 
                                        "HLT_IDTrack_Electron_GSF",
                                        "HLT_IDTrack_Electron_LRTGSF",
                                        "HLT_IDTrack_Muon_FTF", 
                                        "HLT_IDTrack_Muon_IDTrig", 
                                        "HLT_IDTrack_MuonLRT_IDTrig", 
                                        "HLT_IDTrack_MuonIso_FTF", 
                                        "HLT_IDTrack_MuonIso_IDTrig", 
                                        "HLT_IDTrack_MuonLRT_FTF", 
                                        "HLT_IDTrack_Bmumux_FTF", 
                                        "HLT_IDTrack_Bmumux_IDTrig", 
                                        "HLT_IDTrack_TauCore_FTF", 
                                        "HLT_IDTrack_TauLRT_FTF", 
                                        "HLT_IDTrack_TauIso_FTF", 
                                        "HLT_IDTrack_Tau_IDTrig", 
                                        "HLT_IDTrack_TauLRT_IDTrig", 
                                        "HLT_IDTrack_FS_FTF", 
                                        "HLT_IDTrack_FSLRT_FTF", 
                                        "HLT_IDTrack_FSLRT_IDTrig", 
                                        "HLT_IDTrack_DVLRT_FTF", 
                                        "HLT_IDTrack_BeamSpot_FTF", 
                                        "HLT_IDTrack_JetSuper_FTF", 
                                        "HLT_IDTrack_Bjet_FTF", 
                                        "HLT_IDTrack_Bjet_IDTrig", 
                                        "HLT_IDTrack_MinBias_IDTrig", 
                                        "HLT_IDTrack_Cosmic_FTF", 
                                        "HLT_IDTrack_Cosmic_IDTrig", 
                                        "HLT_IDTrack_DJLRT_FTF",
                                        "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtx",
                                        "HLT_IDVertex_FS",
                                        "HLT_IDVertex_JetSuper",
                                        "HLT_IDVertex_Tau",
                                        "HLT_MET_tcpufit",
                                        "HLT_DisTrkBDTSel" ]
    if flags.Tracking.doTrackSegmentsDisappearing:
        TRIG8SlimmingHelper.AllVariables += ["InDetDisappearingTrackParticles"]

    TRIG8SlimmingHelper.StaticContent = [ 
                            "TrigRoiDescriptorCollection#HLT_FSRoI",
                            "TrigRoiDescriptorCollection#HLT_MURoIs",
                            "TrigRoiDescriptorCollection#HLT_eEMRoIs",
                            "TrigRoiDescriptorCollection#HLT_eTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_jTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_cTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_jEMRoIs",
                            "TrigRoiDescriptorCollection#HLT_jJRoIs",
                            "TrigRoiDescriptorCollection#HLT_jLJRoIs",
                            "TrigRoiDescriptorCollection#HLT_gJRoIs",
                            "TrigRoiDescriptorCollection#HLT_gLJRoIs",
                            "TrigRoiDescriptorCollection#HLT_EMRoIs",
                            "TrigRoiDescriptorCollection#HLT_METRoI",
                            "TrigRoiDescriptorCollection#HLT_JETRoI",
                            "TrigRoiDescriptorCollection#HLT_TAURoI",
                            "TrigRoiDescriptorCollection#HLT_Roi_LArPEBHLT",
                            "TrigRoiDescriptorCollection#HLT_Roi_IDCalibPEB",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastElectron",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastElectron_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastElectron_LRT",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastElectron_LRT_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastPhoton",
                            "TrigRoiDescriptorCollection#HLT_Roi_FastPhoton_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_Bmumux",
                            "TrigRoiDescriptorCollection#MuonCandidates_FS_ROIs",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_LRT",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_LRT_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuonForEF",
                            "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuonForEF_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_MuonIso",
                            "TrigRoiDescriptorCollection#HLT_Roi_MuonIso_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_Tau",
                            "TrigRoiDescriptorCollection#HLT_Roi_Tau_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauCore",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauCore_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauLRT",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauLRT_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIso",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIso_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIsoBDT",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIsoBDT_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_JetPEBPhysicsTLA",
                            "TrigRoiDescriptorCollection#HLT_Roi_DV",
                            "TrigRoiDescriptorCollection#HLT_Roi_Bjet",
                            "TrigRoiDescriptorCollection#HLT_Roi_FS",
                            "TrigRoiDescriptorCollection#HLT_Roi_JetSuper",
                            "TrigRoiDescriptorCollection#HLT_Roi_DJ",
                            "TrigInDetTrackTruthMap#TrigInDetTrackTruthMap" ]

    TRIG8SlimmingHelper.ExtraVariables += [ 
                        "Electrons.Tight.Medium.Loose.LHTight.LHMedium.LHLoose",
                        "egammaClusters.phi_sampl.eta0.phi0",
                        "TruthPrimaryVertices.t.x.y.z",
                        "PrimaryVertices.t.x.y.z.numberDoF.chiSquared.covariance.trackParticleLinks",
                        "InDetTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.trackFitter.patternRecoInfo.numberDoF.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "GSFTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.trackFitter.patternRecoInfo.hitPattern.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                        "TauJets.dRmax.etOverPtLeadTrk",
                        "HLT_AntiKt4EMTopoJets_subjesIS.m.pt.eta.phi"]
    if flags.Tracking.doLargeD0:
        TRIG8SlimmingHelper.ExtraVariables += [
            "LRTElectrons.Tight.Medium.Loose.LHTight.LHMedium.LHLoose",
            "LRTegammaClusters.phi_sampl.eta0.phi0",
            "InDetLargeD0TrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.trackFitter.patternRecoInfo.numberDoF.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
            "LRTGSFTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.trackFitter.patternRecoInfo.hitPattern.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit"]

    # Truth containers
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(TRIG8SlimmingHelper)
        TRIG8SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        TRIG8SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                               "Muons.TruthLink",
                                               "Photons.TruthLink"]
        if flags.Tracking.doLargeD0:
            TRIG8SlimmingHelper.ExtraVariables += ["LRTElectrons.TruthLink",
                                                   "MuonsLRT.TruthLink"]

    # Trigger content
    TRIG8SlimmingHelper.IncludeTriggerNavigation = True
    TRIG8SlimmingHelper.IncludeAdditionalTriggerContent = True
    TRIG8SlimmingHelper.IncludeJetTriggerContent = False
    TRIG8SlimmingHelper.IncludeMuonTriggerContent = False
    TRIG8SlimmingHelper.IncludeEGammaTriggerContent = False
    TRIG8SlimmingHelper.IncludeTauTriggerContent = False
    TRIG8SlimmingHelper.IncludeEtMissTriggerContent = False
    TRIG8SlimmingHelper.IncludeBJetTriggerContent = False
    TRIG8SlimmingHelper.IncludeBPhysTriggerContent = False
    TRIG8SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerMatchingToolRun2Cfg
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG8SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG8SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau)

        if flags.Tracking.doLargeD0:
            # Schedule additional pre-matching against LLP offline muons and electrons
            acc.merge(LLP1TriggerMatchingToolRun2Cfg(
                flags,
                name = "LRTTriggerMatchingTool",
                OutputContainerPrefix = "LRTTrigMatch_",
                TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau,
                InputElectrons=TRIG8MergedElectronContainer,
                InputMuons=TRIG8MergedMuonContainer))
            # And add the additional LLP trigger matching branches to the slimming helper
            AddRun2TriggerMatchingToSlimmingHelper(
                SlimmingHelper = TRIG8SlimmingHelper,
                OutputContainerPrefix = "LRTTrigMatch_",
                TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau,
                InputElectrons=TRIG8MergedElectronContainer,
                InputMuons=TRIG8MergedMuonContainer)

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TRIG8SlimmingHelper)

    # Output stream
    TRIG8ItemList = TRIG8SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TRIG8", ItemList=TRIG8ItemList, AcceptAlgs=["TRIG8Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TRIG8", AcceptAlgs=["TRIG8Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

