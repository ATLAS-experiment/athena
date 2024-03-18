# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# ====================================================================
# PIXELVALID.py
# Component accumulator version - replaces PixelVALID
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be
# run from Reco_tf
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Constants import INFO

# Main algorithm config


def PixelVALIDKernelCfg(flags, name='PixelVALIDKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PIXELVALID"""
    acc = ComponentAccumulator()
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel

    acc.addSequence(seqAND('PixelVALIDSequence'))

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    augmentationTools = []
    tsos_augmentationTools = []

    # Add unbiased track parameters to track particles
    from DerivationFrameworkInDet.InDetToolsConfig import (
        TrackToVertexWrapperCfg)
    PixelVALIDTrackToVertexWrapper = acc.getPrimaryAndMerge(
        TrackToVertexWrapperCfg(flags,
                                name="PixelVALIDTrackToVertexWrapper",
                                DecorationPrefix="PixelVALID"))
    augmentationTools.append(PixelVALIDTrackToVertexWrapper)

    from DerivationFrameworkInDet.InDetToolsConfig import (UsedInVertexFitTrackDecoratorCfg)
    PixelVALIDUsedInFitDecorator = acc.getPrimaryAndMerge(UsedInVertexFitTrackDecoratorCfg(flags,
                                                                                           name="PixelVALIDUsedInFitDecorator"))
    augmentationTools.append(PixelVALIDUsedInFitDecorator)

    # @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
    from DerivationFrameworkInDet.InDetToolsConfig import (TrackParametersAtPVCfg)
    DFCommonZ0AtPV = acc.getPrimaryAndMerge(TrackParametersAtPVCfg(flags,
                                                                   name="DFCommonZ0AtPV",
                                                                   Z0SGEntryName="PixelVALIDInDetTrackZ0AtPV"))
    augmentationTools.append(DFCommonZ0AtPV)

    from DerivationFrameworkInDet.InDetToolsConfig import (TrackStateOnSurfaceDecoratorCfg)
    DFTSOS = acc.getPrimaryAndMerge(TrackStateOnSurfaceDecoratorCfg(flags,
                                                                    name              = "DFTrackStateOnSurfaceDecorator",
                                                                    IsSimulation      = flags.Input.isMC,
                                                                    DecorationPrefix  = "",
                                                                    StorePixel        = True,
                                                                    StoreSCT          = False,
                                                                    StoreTRT          = False,
                                                                    AddExtraEventInfo = False,
                                                                    PRDtoTrackMap     = "",
                                                                    OutputLevel=INFO))
    tsos_augmentationTools.append(DFTSOS)

    from DerivationFrameworkInDet.PixelNtupleMakerConfig import (EventInfoPixelModuleStatusMonitoringCfg)
    DFEI = acc.getPrimaryAndMerge(EventInfoPixelModuleStatusMonitoringCfg(flags,
                                                                          name = "EventInfoPixelModuleStatusMonitoring",
                                                                          OutputLevel =INFO))
    augmentationTools.append(DFEI)

    PixelStoreMode = flags.InDet.PixelDumpMode
    if flags.InDet.PixelDumpMode==3:
        PixelStoreMode = 1

    from DerivationFrameworkInDet.PixelNtupleMakerConfig import (PixelNtupleMakerCfg)
    PixelMonitoringTool = acc.getPrimaryAndMerge(PixelNtupleMakerCfg(flags,
                                                                     name          = "PixelMonitoringTool",
                                                                     StoreMode     = PixelStoreMode))
    tsos_augmentationTools.append(PixelMonitoringTool)

    # ====================================================================
    # SKIMMING TOOLS
    # ====================================================================
    skimmingTools = []
    if flags.InDet.DRAWZSelection:
        sel_muon1  = 'Muons.pt > 25*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
        sel_muon2  = 'Muons.pt > 20*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
        draw_zmumu = '( count (  DRZmumuMass > 70*GeV   &&  DRZmumuMass < 110*GeV ) >= 1 )'
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (InvariantMassToolCfg,xAODStringSkimmingToolCfg,FilterCombinationANDCfg)
        PixelVALID_ZmumuMass = acc.getPrimaryAndMerge(InvariantMassToolCfg(flags, 
                                                                           name="PixelVALID_ZmumuMass",
                                                                           ContainerName            = "Muon",
                                                                           ObjectRequirements       = sel_muon1,
                                                                           SecondObjectRequirements = sel_muon2,
                                                                           MassHypothesis           = 105.66,
                                                                           SecondMassHypothesis     = 105.66, 
                                                                           StoreGateEntryName       = "ZmumuMass"))
        PixelVALID_SkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags, 
                                                                                   name="PixelVALID_SkimmingTool",
                                                                                   expression=draw_zmumu))
        PixelVALID_ANDTool = acc.getPrimaryAndMerge(FilterCombinationANDCfg(flags, 
                                                                            name="PixelVALID_ANDTool",
                                                                            FilterList=[PixelVALID_ZmumuMass,PixelVALID_SkimmingTool]))
        skimmingTools.append(PixelVALID_ANDTool)


    if flags.InDet.PixelDumpMode==3:
        sel_mu = '(Muons.pt > 29*GeV) && Muons.passesIDCuts && (Muons.pt < 1450.0*GeV) '
        muRequirement = '( count( '+sel_mu+'  ) == 1 )'
        sel_tau        = '(TauJets.pt > 30.0*GeV) && (TauJets.RNNJetScoreSigTrans>0.55) && ( TauJets.nTracks == 3)'
        tauRequirement = '( count( '+sel_tau+'  ) == 1 )'
        draw_taumuh =  muRequirement+' && '+tauRequirement
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (xAODStringSkimmingToolCfg)
        PixelVALID_MUTAUH = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags, 
                                                                             name="PixelVALID_MUTAUH",
                                                                             expression=draw_taumuh))
        skimmingTools.append(PixelVALID_MUTAUH)



    PixelVALIDKernelPresel = DerivationKernel("PixelVALIDKernelPresel",
                                              SkimmingTools=skimmingTools)
    acc.addEventAlgo(PixelVALIDKernelPresel, sequenceName="PixelVALIDSequence")

    from InDetConfig.TrackRecoConfig import (ClusterSplitProbabilityContainerName)
    from InDetConfig.InDetPrepRawDataToxAODConfig import (InDetPixelPrepDataToxAODCfg)
    acc.merge(InDetPixelPrepDataToxAODCfg(flags,
                                          name         = "xAOD_Pixel_PrepDataToxAOD",
                                          OutputLevel  = INFO,
                                          ClusterSplitProbabilityName=(ClusterSplitProbabilityContainerName(flags)),
                                          UseTruthInfo = flags.Input.isMC))

    # ====================================================================
    # THINNING TOOLS
    # ====================================================================
    thinningTools = [] 

    # MC truth thinning
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (MenuTruthThinningCfg)
        PixelVALIDTruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(flags,
                                                                                  name                = "PixelVALIDTruthThinningTool",
                                                                                  StreamName          = kwargs['StreamName'],
                                                                                  WriteEverything     = True,
                                                                                  WriteFirstN         = -1,
                                                                                  PreserveAncestors   = True,
                                                                                  PreserveGeneratorDescendants=True))
        thinningTools.append(PixelVALIDTruthThinningTool)

    if flags.InDet.PixelDumpMode==3:
        tau_thinning_expression = "(TauJets.ptFinalCalib>=30.*GeV) && (TauJets.RNNJetScoreSigTrans>0.90) && ( TauJets.nTracks==3)"
        muon_thinning_expression = "(Muons.pt>29*GeV) && Muons.passesIDCuts && (Muons.pt<1450.0*GeV)" 
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (GenericObjectThinningCfg)
        PixelVALID_TauJetsThinning = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                     name            = "PixelVALID_TauJetsThinning",
                                                                                     StreamName      = kwargs['StreamName'],
                                                                                     ContainerName   = "TauJets",
                                                                                     SelectionString = tau_thinning_expression))
        thinningTools.append(PixelVALID_TauJetsThinning)

        PixelVALID_MuonThinning = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                  name            = "PixelVALID_MuonThinning",
                                                                                  StreamName      = kwargs['StreamName'],
                                                                                  ContainerName   = "Muons",
                                                                                  SelectionString = muon_thinning_expression))
        thinningTools.append(PixelVALID_MuonThinning)

    # ====================================================================
    # CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
    # ====================================================================
    acc.addEventAlgo(DerivationKernel(name              = "DFTSOSKernel",
                                      AugmentationTools = tsos_augmentationTools,
                                      ThinningTools     = [],
                                      OutputLevel       =INFO), 
                                      sequenceName="PixelVALIDSequence")

    acc.addEventAlgo(DerivationKernel(name,
                                      AugmentationTools = augmentationTools,
                                      SkimmingTools     = skimmingTools,
                                      ThinningTools     = [],
                                      RunSkimmingFirst  = True,
                                      OutputLevel=INFO), 
                                      sequenceName="PixelVALIDSequence")

    acc.addEventAlgo(DerivationKernel(name="PixelVALIDThinningKernel",
                                      AugmentationTools = [],
                                      ThinningTools     = thinningTools,
                                      OutputLevel       = INFO), 
                                      sequenceName="PixelVALIDSequence")

    return acc

# Main config
def PixelVALIDCfg(flags):
    """Main config fragment for PixelVALID"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(PixelVALIDKernelCfg(flags, 
                                  name       = "PixelVALIDKernel",
                                  StreamName = 'StreamDAOD_PixelVALID'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    PixelVALIDSlimmingHelper = SlimmingHelper("PixelVALIDSlimmingHelper",
                                              NamesAndTypes = flags.Input.TypedCollections,
                                              flags         = flags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    PixelStoreMode = flags.InDet.PixelDumpMode
    if flags.InDet.PixelDumpMode==3:
        PixelStoreMode = 1

    if PixelStoreMode==1:
        PixelVALIDSlimmingHelper.AppendToDictionary.update({
            "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
            "Muons": "xAOD::MuonContainer", "MuonsAux": "xAOD::MuonAuxContainer",
            "Electrons": "xAOD::ElectronContainer",
            "ElectronsAux": "xAOD::ElectronAuxContainer",
            "Photons": "xAOD::PhotonContainer",
            "PhotonsAux": "xAOD::PhotonAuxContainer",
            "JetETMissNeutralParticleFlowObjects": "xAOD::FlowElementContainer",
            "JetETMissNeutralParticleFlowObjectsAux": "xAOD::FlowElementAuxContainer",
            "JetETMissChargedParticleFlowObjects": "xAOD::FlowElementContainer",
            "JetETMissChargedParticleFlowObjectsAux": "xAOD::FlowElementAuxContainer",
            "TauJets": "xAOD::TauJetContainer",
            "TauJetsAux": "xAOD::TauJetAuxContainer",
            "InDetTrackParticles": "xAOD::TrackParticleContainer",
            "InDetTrackParticlesAux": "xAOD::TrackParticleAuxContainer",
            "InDetLargeD0TrackParticles": "xAOD::TrackParticleContainer",
            "InDetLargeD0TrackParticlesAux": "xAOD::TrackParticleAuxContainer",
            "Kt4EMTopoOriginEventShape": "xAOD::EventShape",
            "Kt4EMTopoOriginEventShapeAux": "xAOD::EventShapeAuxInfo",
            "Kt4LCTopoOriginEventShape": "xAOD::EventShape",
            "Kt4LCTopoOriginEventShapeAux": "xAOD::EventShapeAuxInfo",
            "NeutralParticleFlowIsoCentralEventShape": "xAOD::EventShape",
            "NeutralParticleFlowIsoCentralEventShapeAux": "xAOD::EventShapeAuxInfo",
            "NeutralParticleFlowIsoForwardEventShape": "xAOD::EventShape",
            "NeutralParticleFlowIsoForwardEventShapeAux": "xAOD::EventShapeAuxInfo",
            "TopoClusterIsoCentralEventShape": "xAOD::EventShape",
            "TopoClusterIsoCentralEventShapeAux": "xAOD::EventShapeAuxInfo",
            "TopoClusterIsoForwardEventShape": "xAOD::EventShape",
            "TopoClusterIsoForwardEventShapeAux": "xAOD::EventShapeAuxInfo",
            "MET_Calo": "xAOD::MissingETContainer",
            "MET_CaloAux": "xAOD::MissingETAuxContainer",
            "MET_Track": "xAOD::MissingETContainer",
            "MET_TrackAux": "xAOD::MissingETAuxContainer",
            "MET_LocHadTopo": "xAOD::MissingETContainer",
            "MET_LocHadTopoRegions": "xAOD::MissingETContainer",
            "MET_LocHadTopoAux": "xAOD::MissingETAuxContainer",
            "MET_LocHadTopoRegionsAux": "xAOD::MissingETAuxContainer",
            "MET_Core_AntiKt4LCTopo": "xAOD::MissingETContainer",
            "MET_Reference_AntiKt4LCTopo": "xAOD::MissingETContainer",
            "MET_Core_AntiKt4LCTopoAux": "xAOD::MissingETAuxContainer",
            "MET_Reference_AntiKt4LCTopoAux": "xAOD::MissingETAuxContainer"})

        SmartCollections += ["Muons", "Electrons", "Photons"]

        AllVariables += ["EventInfo",
                         "JetETMissNeutralParticleFlowObjects",
                         "JetETMissChargedParticleFlowObjects",
                         "InDetTrackParticles",
                         "InDetLargeD0TrackParticles",
                         "Kt4EMTopoOriginEventShape",
                         "Kt4LCTopoOriginEventShape",
                         "NeutralParticleFlowIsoCentralEventShape",
                         "NeutralParticleFlowIsoForwardEventShape",
                         "TopoClusterIsoCentralEventShape",
                         "TopoClusterIsoForwardEventShape"]

        PixelVALIDSlimmingHelper.AppendToDictionary.update({
            "TauJets": "xAOD::TauJetContainer",
            "TauJetsAux": "xAOD::TauJetAuxContainer",
            "Kt4EMPFlowEventShape": "xAOD::EventShape",
            "Kt4EMPFlowEventShapeAux": "xAOD::EventShapeAuxInfo",
            "PrimaryVertices": "xAOD::VertexContainer",
            "PrimaryVerticesAux": "xAOD::VertexAuxContainer",
            "AntiKt4EMTopoJets": "xAOD::JetContainer",
            "AntiKt4EMTopoJetsAux": "xAOD::JetAuxContainer",
            "AntiKt4EMPFlowJets": "xAOD::JetContainer",
            "AntiKt4EMPFlowJetsAux": "xAOD::JetAuxContainer",
            "BTagging_AntiKt4EMTopo": "xAOD::BTaggingContainer",
            "BTagging_AntiKt4EMTopoAux": "xAOD::BTaggingAuxContainer",
            "BTagging_AntiKt4EMPFlow": "xAOD::BTaggingContainer",
            "BTagging_AntiKt4EMPFlowAux": "xAOD::BTaggingAuxContainer"})

        ExtraVariables += ["TauJets.ABS_ETA_LEAD_TRACK.ClusterTotalEnergy.ClustersMeanCenterLambda.ClustersMeanEMProbability.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.ClustersMeanSecondLambda.EMFRACTIONATEMSCALE_MOVEE3.EMFracFixed.GhostMuonSegmentCount.LeadClusterFrac.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.NNDecayModeProb_3pXn.PFOEngRelDiff.PanTau_DecayModeExtended.TAU_ABSDELTAETA.TAU_ABSDELTAPHI.TAU_SEEDTRK_SECMAXSTRIPETOVERPT.UpsilonCluster.absipSigLeadTrk.chargedFELinks.etHotShotDR1.etHotShotDR1OverPtLeadTrk.etHotShotWin.etHotShotWinOverPtLeadTrk.etaCombined.hadLeakFracFixed.leadTrackProbHT.mCombined.mu.nConversionTracks.nFakeTracks.nModifiedIsolationTracks.nVtxPU.neutralFELinks.passThinning.phiCombined.ptCombined.ptIntermediateAxisEM.rho"]
        ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

        AllVariables += ["Kt4EMPFlowEventShape",
                         "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                         "BTagging_AntiKt4EMTopo", "BTagging_AntiKt4EMPFlow"]

        if flags.Input.isMC:
            PixelVALIDSlimmingHelper.AppendToDictionary.update({
                "AntiKt4TruthJets": "xAOD::JetContainer",
                "AntiKt4TruthJetsAux": "xAOD::JetAuxContainer",
                "JetInputTruthParticles": "xAOD::TruthParticleContainer",
                "JetInputTruthParticlesNoWZ": "xAOD::TruthParticleContainer",
                "TruthEvents": "xAOD::TruthEventContainer",
                "TruthEventsAux": "xAOD::TruthEventAuxContainer",
                "TruthParticles": "xAOD::TruthParticleContainer",
                "TruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "egammaTruthParticles": "xAOD::TruthParticleContainer",
                "egammaTruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "MuonTruthParticles": "xAOD::TruthParticleContainer",
                "MuonTruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "LRTegammaTruthParticles": "xAOD::TruthParticleContainer",
                "LRTegammaTruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "TruthVertices": "xAOD::TruthVertexContainer",
                "TruthVerticesAux": "xAOD::TruthVertexAuxContainer",
                "MET_Truth": "xAOD::MissingETContainer",
                "MET_TruthRegions": "xAOD::MissingETContainer",
                "MET_TruthAux": "xAOD::MissingETAuxContainer",
                "MET_TruthRegionsAux": "xAOD::MissingETAuxContainer"})

            AllVariables += ["AntiKt4TruthJets",
                             "JetInputTruthParticles",
                             "JetInputTruthParticlesNoWZ",
                             "TruthEvents",
                             "TruthParticles",
                             "egammaTruthParticles",
                             "MuonTruthParticles",
                             "LRTegammaTruthParticles",
                             "TruthVertices"]

            list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",
                        "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                        "HBosons", "Partons", "TQuarksFinal", "TausFinal",
                        "WBosons", "ZBosons"]
            for item in list_aux:
                label = "TruthLabel"+item
                labelAux = label+"Aux"
                PixelVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

        # Trigger info is actually stored only when running on data...
        PixelVALIDSlimmingHelper.IncludeTriggerNavigation = True
        PixelVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True

    if PixelStoreMode==2:
        PixelVALIDSlimmingHelper.AppendToDictionary.update({
            "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
            "PixelMonitoringTrack": "xAOD::TrackParticleContainer",
            "PixelMonitoringTrackAux": "xAOD::TrackParticleAuxContainer"})

        AllVariables += ["EventInfo",
                         "PixelMonitoringTrack"]

        if flags.Input.isMC:
            PixelVALIDSlimmingHelper.AppendToDictionary.update({
                "TruthEvents": "xAOD::TruthEventContainer",
                "TruthEventsAux": "xAOD::TruthEventAuxContainer",
                "TruthParticles": "xAOD::TruthParticleContainer",
                "TruthParticlesAux": "xAOD::TruthParticleAuxContainer"})

            AllVariables += ["TruthEvents",
                             "TruthParticles"]

            list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",
                        "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                        "HBosons", "Partons", "TQuarksFinal", "TausFinal",
                        "WBosons", "ZBosons"]
            for item in list_aux:
                label = "TruthLabel"+item
                labelAux = label+"Aux"
                PixelVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

    PixelVALIDSlimmingHelper.AllVariables = AllVariables
    PixelVALIDSlimmingHelper.StaticContent = StaticContent
    PixelVALIDSlimmingHelper.SmartCollections = SmartCollections
    PixelVALIDSlimmingHelper.ExtraVariables = ExtraVariables

    # Output stream
    PixelVALIDItemList = PixelVALIDSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_PIXELVALID",
        ItemList=PixelVALIDItemList, AcceptAlgs=["PixelVALIDKernel"]))

    acc.merge(SetupMetaDataForStreamCfg(
        flags, "DAOD_PIXELVALID", AcceptAlgs=["PixelVALIDKernel"],
        createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
