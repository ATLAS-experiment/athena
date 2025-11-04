# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# IDTRKVALID.py
# Component accumulator version - replaces IDTRKVALID
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be
# run from Reco_tf
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Constants import INFO

IDTRKVALID_AOD_EXCLUDED_AUXDATA = ['IDTRKVALID_'+var for var in [
        'biased_PVz0Sigma',
        'unbiased_PVz0Sigma',
        'biased_PVd0Sigma',
        'unbiased_PVd0Sigma',
        'biased_d0Sigma',
        'unbiased_z0',
        'unbiased_d0Sigma',
        'biased_PVz0SigmaSinTheta',
        'biased_z0',
        'unbiased_PVz0SigmaSinTheta',
        'biased_z0Sigma',
        'biased_z0SigmaSinTheta',
        'unbiased_d0',
        'unbiased_z0Sigma',
        'unbiased_z0SigmaSinTheta',
        'biased_d0',
        'biased_z0SinTheta',
        'unbiased_z0SinTheta' ] ]

# Main algorithm config

def IDTRKVALID_ANDToolCfg(flags, name='IDTRKVALID_ANDTool'):
    acc = ComponentAccumulator()

    sel_muon1  = 'Muons.pt > 25*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
    sel_muon2  = 'Muons.pt > 20*GeV && Muons.ptcone40/Muons.pt < 0.3 && Muons.passesIDCuts'
    draw_zmumu = '( count (  DRZmumuMass > 70*GeV   &&  DRZmumuMass < 110*GeV ) >= 1 )'
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        InvariantMassToolCfg, xAODStringSkimmingToolCfg,
        FilterCombinationANDCfg)
    IDTRKVALID_ZmumuMass = acc.getPrimaryAndMerge(InvariantMassToolCfg(
        flags, name="IDTRKVALID_ZmumuMass",
        ContainerName            = "Muon",
        ObjectRequirements       = sel_muon1,
        SecondObjectRequirements = sel_muon2,
        MassHypothesis           = 105.66,
        SecondMassHypothesis     = 105.66,
        StoreGateEntryName       = "DRZmumuMass",
        InputDecorNames          = ["Muons.ptcone40"]))

    IDTRKVALID_SkimmingTool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(flags, name="IDTRKVALID_SkimmingTool",
                                  expression=draw_zmumu))

    IDTRKVALID_ANDTool = acc.getPrimaryAndMerge(
        FilterCombinationANDCfg(flags, name,
                                FilterList=[IDTRKVALID_ZmumuMass,
                                            IDTRKVALID_SkimmingTool]))

    acc.addPublicTool(IDTRKVALID_ANDTool, primary=True)
    return acc


def IDTRKVALIDKernelCommonCfg(flags, name='IDTRKVALIDKernel'):
    acc = ComponentAccumulator()

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    augmentationTools = []

    if flags.Tracking.doVertexFinding:
        # Add unbiased track parameters to track particles
        from DerivationFrameworkInDet.InDetToolsConfig import (
            TrackToVertexWrapperCfg)
        IDTRKVALIDTrackToVertexWrapper = acc.getPrimaryAndMerge(
            TrackToVertexWrapperCfg(
                flags, name="IDTRKVALIDTrackToVertexWrapper",
                DecorationPrefix="IDTRKVALID"))
        augmentationTools.append(IDTRKVALIDTrackToVertexWrapper)

        from DerivationFrameworkInDet.InDetToolsConfig import (
            UsedInVertexFitTrackDecoratorCfg)
        IDTRKVALIDUsedInFitDecorator = acc.getPrimaryAndMerge(
            UsedInVertexFitTrackDecoratorCfg(flags))
        augmentationTools.append(IDTRKVALIDUsedInFitDecorator)

        # @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
        from DerivationFrameworkInDet.InDetToolsConfig import TrackParametersAtPVCfg
        DFCommonZ0AtPV = acc.getPrimaryAndMerge(TrackParametersAtPVCfg(
            flags, name="IDTRKVALID_DFCommonZ0AtPV",
            Z0SGEntryName="IDTRKVALIDInDetTrackZ0AtPV"))
        augmentationTools.append(DFCommonZ0AtPV)

    # ====================================================================
    # SKIMMING TOOLS
    # ====================================================================
    skimmingTools = []
    if flags.InDet.DRAWZSelection:
        IDTRKVALID_ANDTool = acc.getPrimaryAndMerge(IDTRKVALID_ANDToolCfg(flags))
        skimmingTools.append(IDTRKVALID_ANDTool)

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

def IDTRKVALID_PixelModuleStatus_KernelCfg(
        flags, name='IDTRKVALID_PixelModuleStatus_Kernel'):

    acc = ComponentAccumulator()

    augmentationTools = []
    from DerivationFrameworkInDet.PixelNtupleMakerConfig import (
        EventInfoPixelModuleStatusMonitoringCfg)
    DFEI = acc.getPrimaryAndMerge(EventInfoPixelModuleStatusMonitoringCfg(flags))
    augmentationTools.append(DFEI)

    return acc

def IDTRKVALID_ITkPixelModuleStatus_KernelCfg(
        flags, name='IDTRKVALID_ITkPixelModuleStatus_Kernel'):

    acc = ComponentAccumulator()

    augmentationTools = []
    from DerivationFrameworkInDet.PixelNtupleMakerConfig import (
        ITkEventInfoPixelModuleStatusMonitoringCfg)
    DFEI = acc.getPrimaryAndMerge(
        ITkEventInfoPixelModuleStatusMonitoringCfg(flags))
    augmentationTools.append(DFEI)

    return acc

def IDTRKVALIDThinningKernelCfg(
        flags, name="IDTRKVALIDThinningKernel", StreamName=""):
    acc = ComponentAccumulator()

    # ====================================================================
    # THINNING TOOLS
    # ====================================================================
    thinningTools = []

    # MC truth thinning
    if flags.Input.isMC:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            IDTRKVALIDTruthThinningToolCfg)
        thinningTools.append(acc.getPrimaryAndMerge(
            IDTRKVALIDTruthThinningToolCfg(flags, StreamName=StreamName)))

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=[],
        ThinningTools=thinningTools,
        OutputLevel=INFO))
    return acc


def IDTRKVALIDKernelCfg(flags, StreamName=""):
    """Configure the derivation framework driving algorithm (kernel) for IDTRKVALID"""
    acc = ComponentAccumulator()

    IDTRKVALIDSequenceName='IDTRKVALIDSequence'
    acc.addSequence(seqAND(IDTRKVALIDSequenceName))

    acc.merge(IDTRKVALIDKernelCommonCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    acc.merge(IDTRKVALID_PixelModuleStatus_KernelCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPrepDataToxAODCfg
    acc.merge(InDetPrepDataToxAODCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    from DerivationFrameworkInDet.InDetToolsConfig import DFInDetTSOSKernelCfg
    acc.merge(DFInDetTSOSKernelCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    acc.merge(IDTRKVALIDThinningKernelCfg(flags, StreamName=StreamName),
              sequenceName=IDTRKVALIDSequenceName)

    return acc

def ITkTRKVALIDKernelCfg(flags, StreamName=""):
    """Configure the derivation framework driving algorithm (kernel) for IDTRKVALID"""
    acc = ComponentAccumulator()

    IDTRKVALIDSequenceName='IDTRKVALIDSequence'
    acc.addSequence(seqAND(IDTRKVALIDSequenceName))

    acc.merge(IDTRKVALIDKernelCommonCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    acc.merge(IDTRKVALID_ITkPixelModuleStatus_KernelCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    from InDetConfig.InDetPrepRawDataToxAODConfig import ITkPrepDataToxAODCfg
    acc.merge(ITkPrepDataToxAODCfg(flags),
              sequenceName=IDTRKVALIDSequenceName)

    # if the flags.Tracking.writeExtendedSi_PRDInfo is enabled
    # the TSOS_CommonKernelCfg( is scheduled, which would do the same
    # augemntation as the (DFITkTSOSKernel
    if not  flags.Tracking.writeExtendedSi_PRDInfo:
        from DerivationFrameworkInDet.InDetToolsConfig import DFITkTSOSKernelCfg
        acc.merge(DFITkTSOSKernelCfg(flags),
                  sequenceName=IDTRKVALIDSequenceName)

    acc.merge(IDTRKVALIDThinningKernelCfg(flags, StreamName=StreamName),
              sequenceName=IDTRKVALIDSequenceName)

    return acc

# Main config
def IDTRKVALIDCfg(flags):
    """Main config fragment for IDTRKVALID"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    if flags.Detector.GeometryID:
        acc.merge(IDTRKVALIDKernelCfg(flags, StreamName = 'StreamDAOD_IDTRKVALID'))
    if flags.Detector.GeometryITk:
        acc.merge(ITkTRKVALIDKernelCfg(flags, StreamName = 'StreamDAOD_IDTRKVALID'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    IDTRKVALIDSlimmingHelper = SlimmingHelper(
        "IDTRKVALIDSlimmingHelper",
        NamesAndTypes = flags.Input.TypedCollections,
        flags         = flags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
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

    if flags.Detector.GeometryID:
        if flags.InDet.DAODStorePixel:
            IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
                "PixelClusters": "xAOD::TrackMeasurementValidationContainer",
                "PixelClustersAux": "xAOD::TrackMeasurementValidationAuxContainer",
                "PixelMSOSs": "xAOD::TrackStateValidationContainer",
                "PixelMSOSsAux": "xAOD::TrackStateValidationAuxContainer"
            })
        if flags.InDet.DAODStoreSCT:
            IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
                "SCT_Clusters": "xAOD::TrackMeasurementValidationContainer",
                "SCT_ClustersAux": "xAOD::TrackMeasurementValidationAuxContainer",
                "SCT_MSOSs": "xAOD::TrackStateValidationContainer",
                "SCT_MSOSsAux": "xAOD::TrackStateValidationAuxContainer"
            })

    if flags.Detector.GeometryITk:
        if flags.ITk.DAODStorePixel:
            IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
                "ITkPixelMeasurements": "xAOD::TrackMeasurementValidationContainer",
                "ITkPixelMeasurementsAux": "xAOD::TrackMeasurementValidationAuxContainer",
                "ITkPixelMSOSs": "xAOD::TrackStateValidationContainer",
                "ITkPixelMSOSsAux": "xAOD::TrackStateValidationAuxContainer"
            })
        if flags.ITk.DAODStoreStrip:
            IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
                "ITkStripMeasurements": "xAOD::TrackMeasurementValidationContainer",
                "ITkStripMeasurementsAux": "xAOD::TrackMeasurementValidationAuxContainer",
                "ITkStripMSOSs": "xAOD::TrackStateValidationContainer",
                "ITkStripMSOSsAux": "xAOD::TrackStateValidationAuxContainer"
            })

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

    if flags.Detector.GeometryID:
        if flags.InDet.DAODStorePixel:
            AllVariables += ["PixelClusters","PixelMSOSs"]
        if flags.InDet.DAODStoreSCT:
            AllVariables += ["SCT_Clusters","SCT_MSOSs"]
    if flags.Detector.GeometryITk:
        if flags.ITk.DAODStorePixel:
            AllVariables += ["ITkPixelMeasurements","ITkPixelMSOSs"]
        if flags.ITk.DAODStoreStrip:
            AllVariables += ["ITkStripMeasurements","ITkStripMSOSs"]

    IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
        "TauJets": "xAOD::TauJetContainer",
        "TauJetsAux": "xAOD::TauJetAuxContainer",
        "Kt4EMPFlowEventShape": "xAOD::EventShape",
        "Kt4EMPFlowEventShapeAux": "xAOD::EventShapeAuxInfo",
        "PrimaryVertices": "xAOD::VertexContainer",
        "PrimaryVerticesAux": "xAOD::VertexAuxContainer",
        "AntiKt4EMTopoJets": "xAOD::JetContainer",
        "AntiKt4EMTopoJetsAux": "xAOD::JetAuxContainer",
        "AntiKt4EMPFlowJets": "xAOD::JetContainer",
        "AntiKt4EMPFlowJetsAux": "xAOD::JetAuxContainer"})

    ExtraVariables += ["TauJets.ABS_ETA_LEAD_TRACK.ClusterTotalEnergy.ClustersMeanCenterLambda.ClustersMeanEMProbability.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.ClustersMeanSecondLambda.EMFracFixed.GhostMuonSegmentCount.LeadClusterFrac.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.NNDecayModeProb_3pXn.PFOEngRelDiff.PanTau_DecayModeExtended.UpsilonCluster.absipSigLeadTrk.chargedFELinks.etHotShotWin.etHotShotWinOverPtLeadTrk.etaCombined.hadLeakFracFixed.leadTrackProbHT.mCombined.mu.nConversionTracks.nFakeTracks.nModifiedIsolationTracks.nVtxPU.neutralFELinks.passThinning.phiCombined.ptCombined.ptIntermediateAxisEM.rho"]
    ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

    AllVariables += ["Kt4EMPFlowEventShape",
                     "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets"]

    if flags.Input.isMC:
        IDTRKVALIDSlimmingHelper.AppendToDictionary.update({
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
            IDTRKVALIDSlimmingHelper.AppendToDictionary.update(
                {label: "xAOD::TruthParticleContainer",
                 labelAux: "xAOD::TruthParticleAuxContainer"})
            AllVariables += [label]
    # End of isMC block

    # Trigger info is actually stored only when running on data...
    IDTRKVALIDSlimmingHelper.IncludeTriggerNavigation = True
    IDTRKVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True

    IDTRKVALIDSlimmingHelper.AllVariables = AllVariables
    IDTRKVALIDSlimmingHelper.StaticContent = StaticContent
    IDTRKVALIDSlimmingHelper.SmartCollections = SmartCollections
    IDTRKVALIDSlimmingHelper.ExtraVariables = ExtraVariables

    # Output stream
    IDTRKVALIDItemList = IDTRKVALIDSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_IDTRKVALID",
        ItemList=IDTRKVALIDItemList, AcceptAlgs=["IDTRKVALIDKernel"]))

    acc.merge(SetupMetaDataForStreamCfg(
        flags, "DAOD_IDTRKVALID", AcceptAlgs=["IDTRKVALIDKernel"],
        createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
