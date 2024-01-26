# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# ====================================================================
# IDTIDE.py
# Contact: atlas-cp-tracking-denseenvironments@cern.ch
# Component accumulator version - replaces IDTIDE1
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be
# run from Reco_tf
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from AthenaCommon.AlgSequence import AthSequencer as LegacyAthSequencer

from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND,_append
from AthenaCommon.Constants import INFO

# IDTIDE Variables to be excluded from AOD
IDTIDE_AOD_EXCLUDED_AUXDATA = [
    'TrkBLX', 'TrkBLY', 'TrkBLZ', 'TrkIBLX', 'TrkIBLY', 'TrkIBLZ',
    'TrkL1X', 'TrkL1Y', 'TrkL1Z', 'TrkL2X', 'TrkL2Y', 'TrkL2Z',
    'IDTIDE_biased_PVd0Sigma', 'IDTIDE_biased_PVz0Sigma',
    'IDTIDE_biased_PVz0SigmaSinTheta',
    'IDTIDE_biased_d0', 'IDTIDE_biased_d0Sigma',
    'IDTIDE_biased_z0', 'IDTIDE_biased_z0Sigma',
    'IDTIDE_biased_z0SigmaSinTheta', 'IDTIDE_biased_z0SinTheta',
    'IDTIDE_unbiased_PVd0Sigma', 'IDTIDE_unbiased_PVz0Sigma',
    'IDTIDE_unbiased_PVz0SigmaSinTheta',
    'IDTIDE_unbiased_d0', 'IDTIDE_unbiased_d0Sigma',
    'IDTIDE_unbiased_z0', 'IDTIDE_unbiased_z0Sigma',
    'IDTIDE_unbiased_z0SigmaSinTheta', 'IDTIDE_unbiased_z0SinTheta'
    ]


# Main algorithm config
def parSeq(name, subs=[]):
    """ parallel sequencer """
    seq = CompFactory.AthSequencer( name ) if isComponentAccumulatorCfg() else LegacyAthSequencer( name )
    seq.ModeOR = False
    seq.Sequential = False
    seq.StopOverride = True
    for s in subs:
        _append(seq, s)
    return seq

def IDTIDEKernelCommonCfg(flags, name='IDTIDEKernel'):
    acc = ComponentAccumulator()

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    augmentationTools = []

    # Add unbiased track parameters to track particles
    from DerivationFrameworkInDet.InDetToolsConfig import (
        TrackToVertexWrapperCfg)
    IDTIDETrackToVertexWrapper = acc.getPrimaryAndMerge(
        TrackToVertexWrapperCfg(
            flags, name="IDTIDETrackToVertexWrapper",
            DecorationPrefix="IDTIDE")
    )
    augmentationTools.append(IDTIDETrackToVertexWrapper)

    from DerivationFrameworkInDet.InDetToolsConfig import (
        UsedInVertexFitTrackDecoratorCfg)
    IDTIDEUsedInFitDecorator = acc.getPrimaryAndMerge(
        UsedInVertexFitTrackDecoratorCfg(flags))
    augmentationTools.append(IDTIDEUsedInFitDecorator)

    # @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
    from DerivationFrameworkInDet.InDetToolsConfig import (
        TrackParametersAtPVCfg)
    DFCommonZ0AtPV = acc.getPrimaryAndMerge(TrackParametersAtPVCfg(
        flags, name="IDTIDE_DFCommonZ0AtPV",
        Z0SGEntryName="IDTIDEInDetTrackZ0AtPV")
    )
    augmentationTools.append(DFCommonZ0AtPV)

    # ====================================================================
    # SKIMMING TOOLS
    # ====================================================================
    skimmingTools = []
    if not flags.Input.isMC:

        sel_jet600 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 600.*GeV'
        sel_jet800 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 800.*GeV'
        sel_jet1000 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 1000.*GeV'

        desd_jetA = '( HLT_j110_pf_ftf_preselj80_L1J30 || HLT_j175_pf_ftf_preselj140_L1J50 || HLT_j260_pf_ftf_preselj200_L1J75 )'
        desd_jetC = '( HLT_j360_pf_ftf_preselj225_L1J100 )'
        desd_jetD = '( HLT_j420_pf_ftf_preselj225_L1J100 && !HLT_j460_pf_ftf_preselj225_L1J100 )'
        desd_jetE = '( HLT_j460_pf_ftf_preselj225_L1J100 )'
        desd_jetF = '( HLT_j460_pf_ftf_preselj225_L1J100 && count(' + \
            sel_jet600+')>0 && count('+sel_jet800+')==0 )'
        desd_jetG = '( HLT_j460_pf_ftf_preselj225_L1J100 && count(' + \
            sel_jet800+')>0 && count('+sel_jet1000+')==0 )'
        desd_jetH = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet1000+')>0 )'

        prescaleA = 20
        prescaleC = 40
        prescaleD = 30
        prescaleE = 20
        prescaleF = 10
        prescaleG = 5
        # prescaleH = 1 Unused

        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
            xAODStringSkimmingToolCfg,
            PrescaleToolCfg,
            FilterCombinationANDCfg,
            FilterCombinationORCfg)

        IDTIDE_SkimmingToolA = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolA",
                                      expression=desd_jetA))
        IDTIDE_PrescaleToolA = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolA", Prescale=prescaleA))
        IDTIDE_ANDToolA = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolA",
            FilterList=[IDTIDE_SkimmingToolA, IDTIDE_PrescaleToolA]))

        IDTIDE_SkimmingToolC = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolC",
                                      expression=desd_jetC))
        IDTIDE_PrescaleToolC = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolC", Prescale=prescaleC))
        IDTIDE_ANDToolC = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolC",
            FilterList=[IDTIDE_SkimmingToolC, IDTIDE_PrescaleToolC]))

        IDTIDE_SkimmingToolD = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolD",
                                      expression=desd_jetD))
        IDTIDE_PrescaleToolD = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolD", Prescale=prescaleD))
        IDTIDE_ANDToolD = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolD",
            FilterList=[IDTIDE_SkimmingToolD, IDTIDE_PrescaleToolD]))

        IDTIDE_SkimmingToolE = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolE",
                                      expression=desd_jetE))
        IDTIDE_PrescaleToolE = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolE", Prescale=prescaleE))
        IDTIDE_ANDToolE = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolE",
            FilterList=[IDTIDE_SkimmingToolE, IDTIDE_PrescaleToolE]))

        IDTIDE_SkimmingToolF = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolF",
                                      expression=desd_jetF))
        IDTIDE_PrescaleToolF = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolF", Prescale=prescaleF))
        IDTIDE_ANDToolF = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolF",
            FilterList=[IDTIDE_SkimmingToolF, IDTIDE_PrescaleToolF]))

        IDTIDE_SkimmingToolG = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolG",
                                      expression=desd_jetG))
        IDTIDE_PrescaleToolG = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="IDTIDE_PrescaleToolG", Prescale=prescaleG))
        IDTIDE_ANDToolG = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
            flags, name="IDTIDE_ANDToolG",
            FilterList=[IDTIDE_SkimmingToolG, IDTIDE_PrescaleToolG]))

        IDTIDE_SkimmingToolH = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name="IDTIDE_SkimmingToolH",
                                      expression=desd_jetH))

        IDTIDE_ORTool = acc.getPrimaryAndMerge(FilterCombinationORCfg(
            flags, name="IDTIDELogicalCombination",
            FilterList=[IDTIDE_ANDToolA, IDTIDE_ANDToolC, IDTIDE_ANDToolD,
                        IDTIDE_ANDToolE, IDTIDE_ANDToolF,
                        IDTIDE_ANDToolG, IDTIDE_SkimmingToolH]))

        skimmingTools.append(IDTIDE_ORTool)
    # End of: if not flags.Input.isMC

    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            "IDTIDEKernelPresel", SkimmingTools=skimmingTools))

    # ====================================================================
    # CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
    # ====================================================================
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=augmentationTools,
        SkimmingTools=skimmingTools,
        ThinningTools=[],
        RunSkimmingFirst=True,
        OutputLevel=INFO))

    return acc


def IDTIDEThinningKernelCfg(flags, name="IDTIDEThinningKernel", StreamName=""):
    acc = ComponentAccumulator()

    # ====================================================================
    # THINNING TOOLS
    # ====================================================================
    thinningTools = []

    # TrackParticles directly
    if flags.Detector.GeometryID:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            IDTIDEThinningToolCfg)
        thinningTools.append(acc.getPrimaryAndMerge(
            IDTIDEThinningToolCfg(flags, StreamName=StreamName)))
    if flags.Detector.GeometryITk:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            ITkTIDEThinningToolCfg)
        thinningTools.append(acc.getPrimaryAndMerge(
            ITkTIDEThinningToolCfg(flags, StreamName=StreamName)))

    # MC truth thinning
    if flags.Input.isMC:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            IDTIDETruthThinningToolCfg)
        thinningTools.append(acc.getPrimaryAndMerge(
            IDTIDETruthThinningToolCfg(flags, StreamName=StreamName)))

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=[],
        ThinningTools=thinningTools,
        OutputLevel=INFO))
    return acc

def IDTIDEKernelCfg(flags, StreamName=""):
    """Configure the derivation framework driving algorithm (kernel) for IDTIDE"""
    acc = ComponentAccumulator()

    # Sequence for skimming kernel (if running on data) -> PrepDataToxAOD -> IDTIDE kernel
    # sequence to be used for algorithm which should run before the IDTIDEPresel
    # Disabled as currently blocks decoration of Z0 and thus crashes thinning
    IDTIDEPreselSequenceName='IDTIDEPreselSequence'
    acc.addSequence(seqAND(IDTIDEPreselSequenceName))

    acc.merge(IDTIDEKernelCommonCfg(flags), sequenceName=IDTIDEPreselSequenceName)

    # Add decoration with truth parameters if running on simulation
    # No idea what to do with this
    # if flags.Input.isMC:
    #    # add track parameter decorations to truth particles but only if the decorations have not been applied already
    #    import InDetPhysValMonitoring.InDetPhysValDecoration
    #    meta_data = InDetPhysValMonitoring.InDetPhysValDecoration.getMetaData()
    #    from AthenaCommon.Logging import logging
    #    logger = logging.getLogger( "DerivationFramework" )
    #    if len(meta_data) == 0 :
    #        truth_track_param_decor_alg = InDetPhysValMonitoring.InDetPhysValDecoration.getInDetPhysValTruthDecoratorAlg()
    #        if  InDetPhysValMonitoring.InDetPhysValDecoration.findAlg([truth_track_param_decor_alg.getName()]) == None :
    #            IDTIDESequencePre += truth_track_param_decor_alg
    #        else :
    #            logger.info('Decorator %s already present not adding again.' % (truth_track_param_decor_alg.getName() ))
    #    else :
    #        logger.info('IDPVM decorations to track particles already applied to input file not adding again.')

    IDTIDEPreselAlgSequenceName='IDTIDEPreselAlgSequence'
    acc.addSequence(parSeq(IDTIDEPreselAlgSequenceName),
                    parentName=IDTIDEPreselSequenceName)

    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPrepDataToxAODCfg
    acc.merge(InDetPrepDataToxAODCfg(flags),
              sequenceName=IDTIDEPreselAlgSequenceName)

    IDTIDEPostProcSequenceName='IDTIDEPostProcSequence'
    acc.addSequence(parSeq(IDTIDEPostProcSequenceName),
                    parentName=IDTIDEPreselSequenceName)

    from DerivationFrameworkInDet.InDetToolsConfig import (
        DFInDetTSOSKernelCfg)
    acc.merge(DFInDetTSOSKernelCfg(flags),
              sequenceName=IDTIDEPostProcSequenceName)
    acc.merge(IDTIDEThinningKernelCfg(flags, StreamName=StreamName),
              sequenceName=IDTIDEPostProcSequenceName)
    return acc

def ITkTIDEKernelCfg(flags, StreamName=""):
    """Configure the derivation framework driving algorithm (kernel) for IDTIDE"""
    acc = ComponentAccumulator()

    IDTIDEPreselSequenceName='IDTIDEPreselSequence'
    acc.addSequence(seqAND(IDTIDEPreselSequenceName))
    
    acc.merge(IDTIDEKernelCommonCfg(flags), sequenceName=IDTIDEPreselSequenceName)

    IDTIDEPreselAlgSequenceName='IDTIDEPreselAlgSequence'
    acc.addSequence(parSeq(IDTIDEPreselAlgSequenceName),
                    parentName=IDTIDEPreselSequenceName)

    from InDetConfig.InDetPrepRawDataToxAODConfig import ITkPrepDataToxAODCfg
    acc.merge(ITkPrepDataToxAODCfg(flags),
              sequenceName=IDTIDEPreselAlgSequenceName)

    IDTIDEPostProcSequenceName='IDTIDEPostProcSequence'
    acc.addSequence(parSeq(IDTIDEPostProcSequenceName),
                    parentName=IDTIDEPreselSequenceName)

    from DerivationFrameworkInDet.InDetToolsConfig import DFITkTSOSKernelCfg
    acc.merge(DFITkTSOSKernelCfg(flags),
              sequenceName=IDTIDEPostProcSequenceName)
    acc.merge(IDTIDEThinningKernelCfg(flags, StreamName=StreamName),
              sequenceName=IDTIDEPostProcSequenceName)
    return acc

# Main config

def IDTIDECfg(flags):
    """Main config fragment for IDTIDE"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    if flags.Detector.GeometryID:
        acc.merge(IDTIDEKernelCfg(flags, StreamName='StreamDAOD_IDTIDE'))
    if flags.Detector.GeometryITk:
        acc.merge(ITkTIDEKernelCfg(flags, StreamName='StreamDAOD_IDTIDE'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    IDTIDESlimmingHelper = SlimmingHelper(
        "IDTIDESlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        ConfigFlags=flags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    IDTIDESlimmingHelper.AppendToDictionary.update({
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
        "TopoClusterIsoForwardEventShapeAux": "xAOD::EventShapeAuxInfo"}
    )
    if flags.Detector.GeometryID:
        IDTIDESlimmingHelper.AppendToDictionary.update({
            "PixelClusters": "xAOD::TrackMeasurementValidationContainer",
            "PixelClustersAux": "xAOD::TrackMeasurementValidationAuxContainer",
            "SCT_Clusters": "xAOD::TrackMeasurementValidationContainer",
            "SCT_ClustersAux": "xAOD::TrackMeasurementValidationAuxContainer"
        })
    if flags.Detector.GeometryITk:
        IDTIDESlimmingHelper.AppendToDictionary.update({
            "ITkPixelClusters": "xAOD::TrackMeasurementValidationContainer",
            "ITkPixelClustersAux": "xAOD::TrackMeasurementValidationAuxContainer",
            "ITkStripClusters": "xAOD::TrackMeasurementValidationContainer",
            "ITkStripClustersAux": "xAOD::TrackMeasurementValidationAuxContainer"
        })

    SmartCollections += ["Muons", "Electrons", "Photons"]

    AllVariables += ["EventInfo",
                     "JetETMissNeutralParticleFlowObjects",
                     "JetETMissChargedParticleFlowObjects",
                     "InDetTrackParticles",
                     "InDetLargeD0TrackParticles",
                     "PixelClusters",
                     "SCT_Clusters",
                     "Kt4EMTopoOriginEventShape",
                     "Kt4LCTopoOriginEventShape",
                     "NeutralParticleFlowIsoCentralEventShape",
                     "NeutralParticleFlowIsoForwardEventShape",
                     "TopoClusterIsoCentralEventShape",
                     "TopoClusterIsoForwardEventShape",
                     ]
    if flags.Detector.GeometryID:
        AllVariables += ["PixelClusters", "SCT_Clusters"]
    if flags.Detector.GeometryITk:
        AllVariables += ["ITkPixelClusters", "ITkStripClusters"]

    IDTIDESlimmingHelper.AppendToDictionary.update({
        "Kt4EMPFlowEventShape": "xAOD::EventShape",
        "Kt4EMPFlowEventShapeAux": "xAOD::EventShapeAuxInfo",
        "PrimaryVertices": "xAOD::VertexContainer",
        "PrimaryVerticesAux": "xAOD::VertexAuxContainer",
        "InDetTrackParticlesClusterAssociations": "xAOD::TrackParticleClusterAssociationContainer",
        "InDetTrackParticlesClusterAssociationsAux": "xAOD::TrackParticleClusterAssociationAuxContainer",
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
                     "InDetTrackParticlesClusterAssociations",
                     "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                     "BTagging_AntiKt4EMTopo", "BTagging_AntiKt4EMPFlow"]

    if flags.Detector.EnablePixel:
        IDTIDESlimmingHelper.AppendToDictionary.update(
            {'PixelMSOSs': 'xAOD::TrackStateValidationContainer',
             'PixelMSOSsAux': 'xAOD::TrackStateValidationAuxContainer'})
        AllVariables += ["PixelMSOSs"]

    if flags.Detector.EnableSCT:
        IDTIDESlimmingHelper.AppendToDictionary.update(
            {'SCT_MSOSs': 'xAOD::TrackStateValidationContainer',
             'SCT_MSOSsAux': 'xAOD::TrackStateValidationAuxContainer'})
        AllVariables += ["SCT_MSOSs"]

    if flags.Detector.EnableTRT:
        IDTIDESlimmingHelper.AppendToDictionary.update(
            {'TRT_MSOSs': 'xAOD::TrackStateValidationContainer',
             'TRT_MSOSsAux': 'xAOD::TrackStateValidationAuxContainer'})
        AllVariables += ["TRT_MSOSs"]

    if flags.Detector.EnableITkPixel:
        IDTIDESlimmingHelper.AppendToDictionary.update(
            {'ITkPixelMSOSs': 'xAOD::TrackStateValidationContainer',
             'ITkPixelMSOSsAux': 'xAOD::TrackStateValidationAuxContainer'})
        AllVariables += ["ITkPixelMSOSs"]

    if flags.Detector.EnableITkStrip:
        IDTIDESlimmingHelper.AppendToDictionary.update(
            {'ITkStripMSOSs': 'xAOD::TrackStateValidationContainer',
             'ITkStripMSOSsAux': 'xAOD::TrackStateValidationAuxContainer'})
        AllVariables += ["ITkStripMSOSs"]

    if flags.Input.isMC:

        IDTIDESlimmingHelper.AppendToDictionary.update({
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
            "TruthVerticesAux": "xAOD::TruthVertexAuxContainer"})

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
            IDTIDESlimmingHelper.AppendToDictionary.update(
                {label: "xAOD::TruthParticleContainer",
                 labelAux: "xAOD::TruthParticleAuxContainer"})
            AllVariables += [label]
    # End of isMC block

    # Trigger info is actually stored only when running on data...
    IDTIDESlimmingHelper.IncludeTriggerNavigation = True
    IDTIDESlimmingHelper.IncludeAdditionalTriggerContent = True

    IDTIDESlimmingHelper.AllVariables = AllVariables
    IDTIDESlimmingHelper.StaticContent = StaticContent
    IDTIDESlimmingHelper.SmartCollections = SmartCollections
    IDTIDESlimmingHelper.ExtraVariables = ExtraVariables

    # Output stream
    IDTIDEItemList = IDTIDESlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_IDTIDE",
              ItemList=IDTIDEItemList, AcceptAlgs=["IDTIDEKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(
        flags, "DAOD_IDTIDE", AcceptAlgs=["IDTIDEKernel"],
        createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TriggerMenuMetaData]))

    return acc
