# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM16.py
# This defines DAOD_JETM16, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority
# of physics analyses in ATLAS.
# It requires the flag JETM16 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from DerivationFrameworkPhys.PHYSLITE import CPAlgorithmsCfg
from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists


def JETM16KernelCfg(flags, name='JETM16Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM16"""
    acc = ComponentAccumulator()

    # This block does the common physics augmentation  which isn't needed (or possible) for PHYS->JETM16
    # Ensure block only runs for AOD input
    if 'StreamAOD' in flags.Input.ProcessingTags:
        # Common augmentations
        from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
        acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule
    # This differs depending on whether the input is AOD or PHYS
    # These are needed whatever the input since they are not applied in PHYS
    thinningToolsArgs = {
        'ElectronCaloClusterThinningToolName' : "JETM16ElectronCaloClusterThinningTool",
        'PhotonCaloClusterThinningToolName'   : "JETM16PhotonCaloClusterThinningTool",
        'ElectronGSFTPThinningToolName'       : "JETM16ElectronGSFTPThinningTool",
        'PhotonGSFTPThinningToolName'         : "JETM16PhotonGSFTPThinningTool"
    }
    # whereas these are only needed if the input is AOD since they are applied already in PHYS
    if 'StreamAOD' in flags.Input.ProcessingTags:
        thinningToolsArgs.update({
            'MuonTPThinningToolName'              : "JETM16MuonTPThinningTool",
            'TauJetThinningToolName'              : "JETM16TauJetThinningTool",
            'TauJets_MuonRMThinningToolName'      : "JETM16TauJets_MuonRMThinningTool",
            'DiTauTPThinningToolName'             : "JETM16DiTauTPThinningTool",
            'DiTauLowPtThinningToolName'          : "JETM16DiTauLowPtThinningTool",
            'DiTauLowPtTPThinningToolName'        : "JETM16DiTauLowPtTPThinningTool",
        })
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    ####### ADD JETM16 SPECIFIC PORTION ########
    # Add some more track particles, stolen from FTAG1LITE
    from DerivationFrameworkInDet.InDetToolsConfig import (
        EgammaTrackParticleThinningCfg,
        JetConstituentThinningCfg,
        JetTrackParticleThinningCfg,
    )
    # Track quality thinning — mirrors TDD r22loose-track-cuts
    # (TDD still applies all cuts at dump time, so this is purely a DAOD size optimisation)
    stream = kwargs['StreamName']
    track_quality_sel = (
        "InDetTrackParticles.pt > 500"
        " && abs(InDetTrackParticles.eta) < 2.5"
        " && abs(InDetTrackParticles.d0) < 5.0*mm"
        " && (InDetTrackParticles.numberOfPixelHits + InDetTrackParticles.numberOfPixelDeadSensors"
        " + InDetTrackParticles.numberOfSCTHits + InDetTrackParticles.numberOfSCTDeadSensors) >= 8"
        " && (InDetTrackParticles.numberOfPixelHoles + InDetTrackParticles.numberOfSCTHoles) <= 2"
        " && InDetTrackParticles.numberOfPixelHoles <= 1"
    )

    thinningTools.append(acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name="JETM16JetTPThinningTool",
        StreamName=stream,
        JetKey='AntiKt4EMPFlowJets',
        InDetTrackParticlesKey="InDetTrackParticles",
        TrackSelectionString=track_quality_sel,
    )))
    thinningTools.append(acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name="JETM16LargeRJetTPThinningTool",
        StreamName=stream,
        JetKey='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets',
        InDetTrackParticlesKey="InDetTrackParticles",
        TrackSelectionString=track_quality_sel,
    )))

    thinningTools.append(acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name="JETM16ElectronTPThinningTool",
        StreamName=stream,
        SGKey="Electrons",
        InDetTrackParticlesKey="InDetTrackParticles",
    )))

    # Add PFlow Constituents (from FTAG1LITE)
    thinningTools.append(acc.getPrimaryAndMerge(JetConstituentThinningCfg(
        flags,
        name="JETM16JetConstituentThinningTool",
        StreamName=stream,
        JetKey='AntiKt4EMPFlowJets',
        JetConstituentName="CHSG",
        GlobalConstituentName="Global",
        OtherObjectsName="CaloCalTopoClusters",
    )))
    # thinningTools.append(acc.getPrimaryAndMerge(JetConstituentThinningCfg(
    #     flags,
    #     name="JETM16LargeRJetConstituentThinningTool",
    #     StreamName=stream,
    #     JetKey='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets',
    #     JetConstituentName="CHSG",
    #     GlobalConstituentName="Global",
    #     OtherObjectsName="CaloCalTopoClusters",
    # )))
    ####### END JETM16 SPECIFIC PORTION #######


    # Higgs augmentations - 4l vertex, Higgs STXS truth variables, CloseBy isolation correction (for all analyses)
    # For PhysLite, must run CloseBy BEFORE running analysis sequences to be able to 'pass through' to the shallow copy the added isolation values
    # Here we only run the augmentation algs
    # These do not need to be run if PhysLite is run from Phys (i.e. not from 'StreamAOD')
    if 'StreamAOD' in flags.Input.ProcessingTags:
        # running from AOD
        ## Higgs - create 4l vertex
        from DerivationFrameworkHiggs.HiggsPhysContent import  HiggsAugmentationAlgsCfg
        acc.merge(HiggsAugmentationAlgsCfg(flags))

        ## CloseByIsolation correction augmentation
        from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
        acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = True))

    #==============================================================================
    # Analysis-level variables
    #==============================================================================

    # Needed in principle to support MET association when running PHYS->JETM16,
    # but since this doesn't work for PHYS->JETM16 anyway, commenting for now
    #if 'StreamDAOD_PHYS' in flags.Input.ProcessingTags
    #    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    #    acc.merge(GeoModelCfg(flags))

    # add CP algorithms to job
    acc.merge(CPAlgorithmsCfg(flags))

    # Build MET from our analysis objects
    if 'StreamAOD' in flags.Input.ProcessingTags:
        from METReconstruction.METAssocCfg import AssocConfig, METAssocConfig
        from METReconstruction.METAssociatorCfg import getAssocCA
        associators = [AssocConfig('PFlowJet', 'AnalysisJets'),
                       AssocConfig('Muon', 'AnalysisMuons'),
                       AssocConfig('Ele', 'AnalysisElectrons'),
                       AssocConfig('Gamma', 'AnalysisPhotons'),
                       AssocConfig('Tau', 'AnalysisTauJets'),
                       AssocConfig('Soft', '')]
        JETM16_cfg = METAssocConfig('AnalysisMET',
                                      flags,
                                      associators,
                                      doPFlow=True,
                                      usePFOLinks=True)
        components_JETM16_cfg = getAssocCA(JETM16_cfg,METName='AnalysisMET')
        acc.merge(components_JETM16_cfg)
    elif 'StreamDAOD_PHYS' in flags.Input.ProcessingTags:
        from DerivationFrameworkJetEtMiss.METCommonConfig import METRemappingCfg

        METRemap_cfg = METRemappingCfg(flags)
        acc.merge(METRemap_cfg)

    # The derivation kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))

    return acc


def JETM16Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM16TriggerListsHelper = TriggerListsHelper(flags)

    # Set the stream name - varies depending on whether the input is AOD or DAOD_PHYS
    streamName = 'StreamDAOD_JETM16' if 'StreamAOD' in flags.Input.ProcessingTags else 'StreamD2AOD_JETM16'

    # Common augmentations
    acc.merge(JETM16KernelCfg(flags, name="JETM16Kernel", StreamName = streamName, TriggerListsHelper = JETM16TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    JETM16SlimmingHelper = SlimmingHelper("JETM16SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    JETM16SlimmingHelper.ExtraVariables = []
    # Trigger content
    JETM16SlimmingHelper.IncludeTriggerNavigation = False
    JETM16SlimmingHelper.IncludeJetTriggerContent = False
    JETM16SlimmingHelper.IncludeMuonTriggerContent = False
    JETM16SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM16SlimmingHelper.IncludeTauTriggerContent = False
    JETM16SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM16SlimmingHelper.IncludeBJetTriggerContent = False
    JETM16SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM16SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        # Need to re-run matching so that new Analysis<X> containers are matched to triggers
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        acc.merge(TriggerMatchingCommonRun2Cfg(flags,
                                               name = "JETM16TrigMatchNoTau",
                                               OutputContainerPrefix = "AnalysisTrigMatch_",
                                               ChainNames = JETM16TriggerListsHelper.Run2TriggerNamesNoTau,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        acc.merge(TriggerMatchingCommonRun2Cfg(flags,
                                               name = "JETM16TrigMatchTau",
                                               OutputContainerPrefix = "AnalysisTrigMatch_",
                                               ChainNames = JETM16TriggerListsHelper.Run2TriggerNamesTau,
                                               DRThreshold = 0.2,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        # Now add the resulting decorations to the output
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM16SlimmingHelper,
                                         OutputContainerPrefix = "AnalysisTrigMatch_",
                                         TriggerList = JETM16TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM16SlimmingHelper,
                                         OutputContainerPrefix = "AnalysisTrigMatch_",
                                         TriggerList = JETM16TriggerListsHelper.Run2TriggerNamesNoTau)

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        # No need to run matching: just keep navigation so matching can be done by analysts
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM16SlimmingHelper)

    # Event content
    JETM16SlimmingHelper.AppendToDictionary.update({
        'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
        'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
        'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
        'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
        'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
        'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
        'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
        'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
        'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
        'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
        'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
        'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
        'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
        'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
        'AnalysisElectrons':'xAOD::ElectronContainer', 'AnalysisElectronsAux':'xAOD::ElectronAuxContainer',
        'AnalysisSiHitElectrons':'xAOD::ElectronContainer', 'AnalysisSiHitElectronsAux':'xAOD::ElectronAuxContainer',
        'AnalysisMuons':'xAOD::MuonContainer', 'AnalysisMuonsAux':'xAOD::MuonAuxContainer',
        'AnalysisJets':'xAOD::JetContainer','AnalysisJetsAux':'xAOD::AuxContainerBase',
        'AnalysisPhotons':'xAOD::PhotonContainer', 'AnalysisPhotonsAux':'xAOD::PhotonAuxContainer',
        'AnalysisTauJets':'xAOD::TauJetContainer', 'AnalysisTauJetsAux':'xAOD::TauJetAuxContainer',
        'MET_Core_AnalysisMET':'xAOD::MissingETContainer', 'MET_Core_AnalysisMETAux':'xAOD::MissingETAuxContainer',
        'METAssoc_AnalysisMET':'xAOD::MissingETAssociationMap', 'METAssoc_AnalysisMETAux':'xAOD::MissingETAuxAssociationMap',
        'AnalysisLargeRJets':'xAOD::JetContainer','AnalysisLargeRJetsAux':'xAOD::AuxContainerBase'
    })

    JETM16SlimmingHelper.SmartCollections = [
        'EventInfo',
        'InDetTrackParticles',
        'PrimaryVertices',
    ]
    JETM16SlimmingHelper.AllVariables = [
        "CHSGNeutralParticleFlowObjects",
        "CHSGChargedParticleFlowObjects",
    ]

    from DerivationFrameworkMuons.MuonsCommonConfig import MuonVariablesCfg

    # add in extra values for Higgs
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(flags, JETM16SlimmingHelper)

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM16SlimmingHelper)
        # This block is only needed if input is AOD, as it is already done for PHYS->JETM16
        if 'StreamAOD' in flags.Input.ProcessingTags:
            from DerivationFrameworkMCTruth.HFClassificationCommonConfig import HFClassificationCommonCfg
            acc.merge(HFClassificationCommonCfg(flags))

    # Save the extra variables which aren't included by other means
    btag_variables = [f'{flags.BTagging.AK4TaggerName}_p{x}' for x in ['b', 'c', 'u', 'tau']]
    JETM16SlimmingHelper.ExtraVariables += [
        'AnalysisElectrons.trackParticleLinks.f1.pt.eta.phi.charge.author.DFCommonElectronsLHVeryLoose.DFCommonElectronsLHLoose.DFCommonElectronsLHLooseBL.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsLHVeryLooseIsEMValue.DFCommonElectronsLHLooseIsEMValue.DFCommonElectronsLHLooseBLIsEMValue.DFCommonElectronsLHMediumIsEMValue.DFCommonElectronsLHTightIsEMValue.DFCommonElectronsDNNLoose.DFCommonElectronsDNNMedium.DFCommonElectronsDNNTight.DFCommonElectronsDNNVeryLooseNoCF97.DFCommonElectronsDNNMediumNoCF.DFCommonElectronsDNNTightNoCF.DFCommonElectronsECIDS.DFCommonElectronsECIDSResult.topoetcone20.topoetcone20ptCorrection.neflowisol20.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.topoetcone20_CloseByCorr.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.caloClusterLinks.ambiguityLink.TruthLink.truthOrigin.truthType.truthPdgId.firstEgMotherTruthType.firstEgMotherTruthOrigin.firstEgMotherTruthParticleLink.firstEgMotherPdgId.ambiguityType.OQ.Eadded_Lr2.Eadded_Lr3.E_mva_only.DFCommonAddAmbiguity',
        'AnalysisSiHitElectrons.pt.eta.phi.charge.author.topoetcone20_CloseByCorr.DFCommonElectronsLHVeryLoose.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.OQ.truthOrigin.truthType.firstEgMotherTruthType.firstEgMotherTruthOrigin.z0stheta.d0Normalized.nInnerExpPix.clEta.clPhi.E_mva_only',
        'AnalysisPhotons.f1.pt.eta.phi.author.OQ.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMMedium.DFCommonPhotonsIsEMTight.DFCommonPhotonsIsEMTightIsEMValue.DFCommonPhotonsCleaning.DFCommonPhotonsCleaningNoTime.ptcone20.topoetcone20.topoetcone40.topoetcone20ptCorrection.topoetcone40ptCorrection.topoetcone20_CloseByCorr.topoetcone40_CloseByCorr.ptcone20_CloseByCorr.caloClusterLinks.vertexLinks.ambiguityLink.TruthLink.truthOrigin.truthType.Eadded_Lr2.Eadded_Lr3.E_mva_only',
        'GSFTrackParticles.chiSquared.phi.d0.theta.qOverP.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.z0.vz.charge.vertexLink.numberOfPixelHits.numberOfSCTHits.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberOfInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerHits.originalTrackParticle',
        'GSFConversionVertices.trackParticleLinks.x.y.z.px.py.pz.pt1.pt2.neutralParticleLinks.minRfirstHit',
        'egammaClusters.calE.calEta.calPhi.calM.e_sampl.eta_sampl.ETACALOFRAME.PHICALOFRAME.ETA2CALOFRAME.PHI2CALOFRAME.constituentClusterLinks.samplingPattern',
        "AnalysisMuons.{var_string}".format(var_string = ".".join(MuonVariablesCfg(flags))),
        'CombinedMuonTrackParticles.qOverP.d0.z0.vz.phi.theta.truthOrigin.truthType.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfTRTHits.numberOfTRTOutliers.chiSquared.numberDoF',
        'ExtrapolatedMuonTrackParticles.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.truthOrigin.truthType.qOverP.theta.phi',
        'MuonSpectrometerTrackParticles.phi.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.vertexLink.theta.qOverP',
        'InDetForwardTrackParticles.vz.truthType.truthOrigin.numberDoF.numberOfTRTHits.numberOfSCTHoles.theta.numberOfTRTOutliers.numberOfPrecisionLayers.numberOfSCTDeadSensors.numberOfPixelHoles.numberOfSCTHits.numberOfPrecisionHoleLayers.numberOfPixelDeadSensors.phi.numberOfPixelHits.z0.d0.qOverP.chiSquared.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag',
        'AnalysisTauJets.pt.eta.phi.m.ptFinalCalib.etaFinalCalib.ptTauEnergyScale.etaTauEnergyScale.charge.isTauFlags.PanTau_DecayMode.NNDecayMode.RNNJetScoreSigTrans.GNTauScoreSigTrans_v0prune.GNTauVL_v0prune.GNTauL_v0prune.GNTauM_v0prune.GNTauT_v0prune.RNNEleScoreSigTrans_v1.EleRNNLoose_v1.EleRNNMedium_v1.EleRNNTight_v1.trackWidth.passTATTauMuonOLR.tauTrackLinks.vertexLink.truthParticleLink.truthJetLink.IsTruthMatched.truthOrigin.truthType',
        'AnalysisJets.pt.eta.phi.m.constituentLinks.constituentWeights.ConstituentScale.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.NumTrkPt500.SumPtTrkPt500.DetectorEta.JVFCorr.NNJvtPass.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.PartonTruthLabelID.HadronConeExclExtendedTruthLabelID.HadronConeExclTruthLabelID.TrueFlavor.DFCommonJets_jetClean_LooseBad.DFCommonJets_jetClean_TightBad.Timing.btagging.btaggingLink.GhostTrack.DFCommonJets_fJvt.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PSFrac.JetAccessorMap.EMFrac.Width.ActiveArea4vec_pt.ActiveArea4vec_eta.ActiveArea4vec_m.ActiveArea4vec_phi.EnergyPerSampling.SumPtChargedPFOPt500.isJvtHS.{btag_var_string}'.format(btag_var_string = ".".join(btag_variables)),
        'TruthPrimaryVertices.t.x.y.z',
        'MET_Core_AnalysisMET.name.mpx.mpy.sumet.source',
        'METAssoc_AnalysisMET.',
        'InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.numberOfTRTHits.numberOfTRTOutliers',
        'EventInfo.RandomRunNumber.PileupWeight_NOSYS.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification.{GRL_Deco_names}'.format(GRL_Deco_names='.'.join(str(key) for key in (getGoodRunsLists()).keys())),
        'Kt4EMPFlowEventShape.Density',
        'Kt4EMPFlowNeutEventShape.Density',
        'TauTracks.flagSet.trackLinks',
        'AnalysisLargeRJets.pt.eta.phi.m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.DetectorEta.TrackSumMass.TrackSumPt.constituentLinks.ECF1.ECF2.ECF3.Tau1_wta.Tau2_wta.Tau3_wta.Split12.Split23.Qw.D2.C2.R10TruthLabel_R22v1.R10TruthLabel_R21Precision_2022v1.GhostBHadronsFinalCount.GhostCHadronsFinalCount.Parent.GN2Xv01_phbb.GN2Xv01_phcc.GN2Xv01_ptop.GN2Xv01_pqcd',
        ]

    # Output stream
    JETM16ItemList = JETM16SlimmingHelper.GetItemList()

    formatString = 'D2AOD_JETM16' if 'StreamDAOD_PHYS' in flags.Input.ProcessingTags else 'DAOD_JETM16'
    acc.merge(OutputStreamCfg(flags, formatString, ItemList=JETM16ItemList, AcceptAlgs=["JETM16Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, formatString, AcceptAlgs=["JETM16Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc
