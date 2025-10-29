# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# PIXELVALID.py
# Component accumulator version
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

def PIXELVALID_ANDToolCfg(flags, name='PIXELVALID_ANDTool'):
    acc = ComponentAccumulator()

    sel_muon1  = 'Muons.pt > 25*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
    sel_muon2  = 'Muons.pt > 20*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
    draw_zmumu = '( count (  DRZmumuMass > 70*GeV   &&  DRZmumuMass < 110*GeV ) >= 1 ) '
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (InvariantMassToolCfg,xAODStringSkimmingToolCfg,FilterCombinationANDCfg)
    PIXELVALID_ZmumuMass = acc.getPrimaryAndMerge(InvariantMassToolCfg(
        flags, name="PIXELVALID_ZmumuMass",
        ContainerName            = "Muon",
        ObjectRequirements       = sel_muon1,
        SecondObjectRequirements = sel_muon2,
        MassHypothesis           = 105.66,
        SecondMassHypothesis     = 105.66,
        StoreGateEntryName       = "DRZmumuMass",
        InputDecorNames          = ["Muons.ptcone40"]))

    PIXELVALID_SkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags,
        name="PIXELVALID_SkimmingTool",
        expression=draw_zmumu))

    PIXELVALID_ANDTool = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
        flags, 
        name,
        FilterList=[PIXELVALID_ZmumuMass,PIXELVALID_SkimmingTool]))

    acc.addPublicTool(PIXELVALID_ANDTool, primary=True)
    return acc


def PIXELVALID_ZTAUTAUCfg(flags, name='PIXELVALID_ZTAUTAU'):
    acc = ComponentAccumulator()

#    sel_mu = '(Muons.pt > 29*GeV) && Muons.passesIDCuts'
    sel_mu = '(Muons.pt > 10*GeV) && Muons.passesIDCuts'
    muRequirement = '( count( '+sel_mu+'  ) == 1 )'
#    sel_tau       = '(TauJets.pt > 30.0*GeV) && (TauJets.RNNJetScoreSigTrans>0.55) && ( TauJets.nTracks == 3)'
    sel_tau       = '(TauJets.pt > 10.0*GeV)'
#    tauRequirement = '( count( '+sel_tau+'  ) == 1 )'
    tauRequirement = '( count( '+sel_tau+'  ) > 0 )'
    draw_taumuh =  muRequirement+' && '+tauRequirement
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (xAODStringSkimmingToolCfg)
    PIXELVALID_ZTAUTAU = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags, 
                                                                         name="PIXELVALID_ZTAUTAU",
                                                                         expression=draw_taumuh))

    acc.addPublicTool(PIXELVALID_ZTAUTAU, primary=True)
    return acc

    
def PIXELVALIDKernelCommonCfg(flags, name='PIXELVALIDKernelCommon'):
    acc = ComponentAccumulator()

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    augmentationTools = []

    # Add unbiased track parameters to track particles
    from DerivationFrameworkInDet.InDetToolsConfig import (TrackToVertexWrapperCfg)
    PIXELVALIDTrackToVertexWrapper = acc.getPrimaryAndMerge(TrackToVertexWrapperCfg(
        flags,
        name="PIXELVALIDTrackToVertexWrapper",
        DecorationPrefix="PIXELVALID"))
    augmentationTools.append(PIXELVALIDTrackToVertexWrapper)

    from DerivationFrameworkInDet.InDetToolsConfig import (UsedInVertexFitTrackDecoratorCfg)
    PIXELVALIDUsedInFitDecorator = acc.getPrimaryAndMerge(UsedInVertexFitTrackDecoratorCfg(flags))
    augmentationTools.append(PIXELVALIDUsedInFitDecorator)

    # @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParametersAtPVCfg
    DFCommonZ0AtPV = acc.getPrimaryAndMerge(TrackParametersAtPVCfg(
        flags, name="PIXELVALID_DFCommonZ0AtPV",
        Z0SGEntryName="PIXELVALIDInDetTrackZ0AtPV"))
    augmentationTools.append(DFCommonZ0AtPV)

    from DerivationFrameworkInDet.PixelNtupleMakerConfig import (EventInfoPixelModuleStatusMonitoringCfg)
    DFEI = acc.getPrimaryAndMerge(EventInfoPixelModuleStatusMonitoringCfg(flags))
    augmentationTools.append(DFEI)

    # ====================================================================
    # SKIMMING TOOLS
    # ====================================================================
    
    skimmingTools = []
    if flags.InDet.DRAWZSelection:
        PIXELVALID_ANDTool = acc.getPrimaryAndMerge(PIXELVALID_ANDToolCfg(flags))
        skimmingTools.append(PIXELVALID_ANDTool)

    if flags.InDet.PixelDumpMode==3:
        PIXELVALID_ZTAUTAU = acc.getPrimaryAndMerge(PIXELVALID_ZTAUTAUCfg(flags))
        skimmingTools.append(PIXELVALID_ZTAUTAU)

    # ====================================================================
    # CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
    # ====================================================================
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools = augmentationTools,
        SkimmingTools     = skimmingTools,
        ThinningTools     = [],
        RunSkimmingFirst  = True))

    return acc


def PIXELVALIDThinningKernelCfg(flags, name="PIXELVALIDThinningKernel", StreamName=""):
    acc = ComponentAccumulator()

    # ====================================================================
    # THINNING TOOLS
    # ====================================================================
    thinningTools = []

    # MC truth thinning
    if flags.Input.isMC:
        from DerivationFrameworkInDet.InDetToolsConfig import (IDTRKVALIDTruthThinningToolCfg)
        thinningTools.append(acc.getPrimaryAndMerge(IDTRKVALIDTruthThinningToolCfg(flags, StreamName=StreamName)))
        acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
            name,
            AugmentationTools=[],
            ThinningTools=thinningTools,
            OutputLevel=INFO))

    elif flags.InDet.PixelDumpMode == 5:
        acc.addSequence( seqAND("PixelClusterThinningSequence") )
        acc.getSequence("PixelClusterThinningSequence").ExtraDataForDynamicConsumers = ["xAOD::TrackMeasurementValidationContainer/PixelClusters"]
        acc.getSequence("PixelClusterThinningSequence").ProcessDynamicDataDependencies = True
        from DerivationFrameworkInDet.InDetToolsConfig import (PixelClusterThinningCfg)
        thinningTools.append(acc.getPrimaryAndMerge(PixelClusterThinningCfg(flags, StreamName=StreamName)))
        acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
            name,
            AugmentationTools=[],
            ThinningTools=thinningTools,
            OutputLevel=INFO),
            sequenceName="PixelClusterThinningSequence")        

    return acc


def PIXELVALIDKernelCfg(flags, name="PIXELVALIDKernel", StreamName=""):
    """Configure the derivation framework driving algorithm (kernel) for PIXELVALID"""
    acc = ComponentAccumulator()

    PIXELVALIDSequenceName='PIXELVALIDSequence'
    acc.addSequence(seqAND(PIXELVALIDSequenceName))

    acc.merge(PIXELVALIDKernelCommonCfg(flags),sequenceName=PIXELVALIDSequenceName)

    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPrepDataToxAODCfg
    acc.merge(InDetPrepDataToxAODCfg(flags),sequenceName=PIXELVALIDSequenceName)

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    tsos_augmentationTools = []

    from DerivationFrameworkInDet.InDetToolsConfig import DFTrackStateOnSurfaceDecoratorCfg
    DFTSOS = acc.getPrimaryAndMerge(DFTrackStateOnSurfaceDecoratorCfg(flags))
    tsos_augmentationTools.append(DFTSOS)

    PixelStoreMode = flags.InDet.PixelDumpMode
    if flags.InDet.PixelDumpMode==3:
        PixelStoreMode = 1

    ## when using PIXELVALID for ID Lumi, remove this tool which has enormous CPU costs
    if flags.InDet.PixelDumpMode < 4:
        from DerivationFrameworkInDet.PixelNtupleMakerConfig import PixelNtupleMakerCfg
        PixelMonitoringTool = acc.getPrimaryAndMerge(PixelNtupleMakerCfg(flags,
                                                                        name          = "PixelMonitoringTool",
                                                                        StoreMode     = PixelStoreMode))
        tsos_augmentationTools.append(PixelMonitoringTool)

    # shared between IDTIDE and PIXELVALID
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=tsos_augmentationTools,
        ThinningTools=[],
        OutputLevel=INFO))

    acc.merge(PIXELVALIDThinningKernelCfg(flags, StreamName=StreamName), sequenceName=PIXELVALIDSequenceName)

    return acc

# Main config
def PixelVALIDCfg(flags):
    """Main config fragment for PIXELVALID"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    if flags.Detector.GeometryID:
        acc.merge(PIXELVALIDKernelCfg(flags, StreamName = 'StreamDAOD_PIXELVALID'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    PIXELVALIDSlimmingHelper = SlimmingHelper(
        "PIXELVALIDSlimmingHelper",
        NamesAndTypes = flags.Input.TypedCollections,
        flags         = flags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    PixelStoreMode = flags.InDet.PixelDumpMode
    if flags.InDet.PixelDumpMode==3:
        PixelStoreMode = 1

    if flags.InDet.PixelDumpMode == 4:
        PixelStoreMode = 3

    if flags.InDet.PixelDumpMode == 5:
        PixelStoreMode = 4

    if PixelStoreMode==1:
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
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
            "PixelMSOSs": "xAOD::TrackStateValidationContainer",
            "PixelMSOSsAux": "xAOD::TrackStateValidationAuxContainer",
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
                         "PixelMSOSs",
                         "Kt4EMTopoOriginEventShape",
                         "Kt4LCTopoOriginEventShape",
                         "NeutralParticleFlowIsoCentralEventShape",
                         "NeutralParticleFlowIsoForwardEventShape",
                         "TopoClusterIsoCentralEventShape",
                         "TopoClusterIsoForwardEventShape"]

        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
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

        ExtraVariables += ["TauJets.ABS_ETA_LEAD_TRACK.ClusterTotalEnergy.ClustersMeanCenterLambda.ClustersMeanEMProbability.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.ClustersMeanSecondLambda.EMFracFixed.GhostMuonSegmentCount.LeadClusterFrac.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.NNDecayModeProb_3pXn.PFOEngRelDiff.PanTau_DecayModeExtended.UpsilonCluster.absipSigLeadTrk.chargedFELinks.etHotShotWin.etHotShotWinOverPtLeadTrk.etaCombined.hadLeakFracFixed.leadTrackProbHT.mCombined.mu.nConversionTracks.nFakeTracks.nModifiedIsolationTracks.nVtxPU.neutralFELinks.passThinning.phiCombined.ptCombined.ptIntermediateAxisEM.rho"]
        ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

        AllVariables += ["Kt4EMPFlowEventShape",
                         "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                         "BTagging_AntiKt4EMTopo", "BTagging_AntiKt4EMPFlow"]

        if flags.Input.isMC:
            PIXELVALIDSlimmingHelper.AppendToDictionary.update({
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
                PIXELVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

        # Trigger info is actually stored only when running on data...
        PIXELVALIDSlimmingHelper.IncludeTriggerNavigation = True
        PIXELVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True

    if PixelStoreMode==2:
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
            "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
            "PixelMonitoringTrack": "xAOD::TrackParticleContainer",
            "PixelMonitoringTrackAux": "xAOD::TrackParticleAuxContainer"})

        AllVariables += ["EventInfo",
                         "PixelMonitoringTrack"]

        if flags.Input.isMC:
            PIXELVALIDSlimmingHelper.AppendToDictionary.update({
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
                PIXELVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

    ## for ID lumi usage
    if PixelStoreMode == 3:
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
            "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
            "InDetTrackParticles": "xAOD::TrackParticleContainer",
            "InDetTrackParticlesAux": "xAOD::TrackParticleAuxContainer"})

        AllVariables += ["EventInfo",
                         "InDetTrackParticles"]

        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
            "PrimaryVertices": "xAOD::VertexContainer",
            "PrimaryVerticesAux": "xAOD::VertexAuxContainer"})

        ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

        if flags.Input.isMC:
            PIXELVALIDSlimmingHelper.AppendToDictionary.update({
                "TruthEvents": "xAOD::TruthEventContainer",
                "TruthEventsAux": "xAOD::TruthEventAuxContainer",
                "TruthParticles": "xAOD::TruthParticleContainer",
                "TruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "TruthVertices": "xAOD::TruthVertexContainer",
                "TruthVerticesAux": "xAOD::TruthVertexAuxContainer"})

            AllVariables += ["TruthEvents",
                             "TruthParticles",
                             "TruthVertices"]

            list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",
                        "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                        "HBosons", "Partons", "TQuarksFinal", "TausFinal",
                        "WBosons", "ZBosons"]
            for item in list_aux:
                label = "TruthLabel"+item
                labelAux = label+"Aux"
                PIXELVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

        # Trigger info is actually stored only when running on data...
        PIXELVALIDSlimmingHelper.IncludeTriggerNavigation = True
        PIXELVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True

    ## for ID lumi usage: include Pixel Clusters
    if PixelStoreMode == 4:
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
            "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
            "InDetTrackParticles": "xAOD::TrackParticleContainer",
            "InDetTrackParticlesAux": "xAOD::TrackParticleAuxContainer"})

        AllVariables += ["EventInfo",
                         "InDetTrackParticles"]
        
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
                "PixelClusters": "xAOD::TrackMeasurementValidationContainer",
                "PixelClustersAux": "xAOD::TrackMeasurementValidationAuxContainer"})
        
        ExtraVariables += ["PixelClusters.charge.layer.bec.phi_module.eta_module.eta_pixel_index.phi_pixel_index.nRDO.sizePhi.sizeZ.ToT"]
        
        PIXELVALIDSlimmingHelper.AppendToDictionary.update({
            "PrimaryVertices": "xAOD::VertexContainer",
            "PrimaryVerticesAux": "xAOD::VertexAuxContainer"})

        ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

        if flags.Input.isMC:
            PIXELVALIDSlimmingHelper.AppendToDictionary.update({
                "TruthEvents": "xAOD::TruthEventContainer",
                "TruthEventsAux": "xAOD::TruthEventAuxContainer",
                "TruthParticles": "xAOD::TruthParticleContainer",
                "TruthParticlesAux": "xAOD::TruthParticleAuxContainer",
                "TruthVertices": "xAOD::TruthVertexContainer",
                "TruthVerticesAux": "xAOD::TruthVertexAuxContainer"})

            AllVariables += ["TruthEvents",
                             "TruthParticles",
                             "TruthVertices"]

            list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",
                        "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                        "HBosons", "Partons", "TQuarksFinal", "TausFinal",
                        "WBosons", "ZBosons"]

            for item in list_aux:
                label = "TruthLabel"+item
                labelAux = label+"Aux"
                PIXELVALIDSlimmingHelper.AppendToDictionary.update(
                    {label: "xAOD::TruthParticleContainer",
                     labelAux: "xAOD::TruthParticleAuxContainer"})
                AllVariables += [label]
        # End of isMC block

        # Trigger info is actually stored only when running on data...
        PIXELVALIDSlimmingHelper.IncludeTriggerNavigation = True
        PIXELVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True
        
    
    PIXELVALIDSlimmingHelper.AllVariables = AllVariables
    PIXELVALIDSlimmingHelper.StaticContent = StaticContent
    PIXELVALIDSlimmingHelper.SmartCollections = SmartCollections
    PIXELVALIDSlimmingHelper.ExtraVariables = ExtraVariables

    # Output stream
    PIXELVALIDItemList = PIXELVALIDSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_PIXELVALID",
        ItemList=PIXELVALIDItemList, AcceptAlgs=["PIXELVALIDKernelCommon"]))

    if flags.InDet.PixelDumpMode == 4 or flags.InDet.PixelDumpMode == 5:
        acc.merge(SetupMetaDataForStreamCfg(
            flags, "DAOD_PIXELVALID", AcceptAlgs=["PIXELVALIDKernelCommon"],
            createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TriggerMenuMetaData]))
    else:
        acc.merge(SetupMetaDataForStreamCfg(
            flags, "DAOD_PIXELVALID", AcceptAlgs=["PIXELVALIDKernelCommon"],
            createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
