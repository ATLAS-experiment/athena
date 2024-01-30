# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_PHYSLITE.py
# This defines DAOD_PHYSLITE, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag PHYSLITE in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory



def CPAlgorithmsCfg(flags):
    """do the CP algorithm configuration for PHYSLITE"""

    from AthenaCommon.Logging import logging
    logPLCPAlgCfg = logging.getLogger('PLCPAlgCfg')
    logPLCPAlgCfg.info('****************** STARTING PHYSLITE CPAlgorithmsCfg *****************')


    from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
    from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
    configSeq = ConfigSequence ()

    # create factory object to build block configurations
    config = ConfigFactory()
    # get function to make configs
    makeConfig = config.makeConfig

    # Set up the systematics loader/handler algorithm:
    subConfig = makeConfig ('CommonServices')
    subConfig.setOptionValue ('.runSystematics', False)
    configSeq += subConfig

    # Create a pile-up analysis config
    if flags.Input.isMC:
        # setup config and lumicalc files for pile-up tool
        configSeq += makeConfig ('PileupReweighting')

    # set up the muon analysis algorithm config (must come before electrons and photons to allow FSR collection):

    logPLCPAlgCfg.info('Do Muons')

    subConfig = makeConfig ('Muons', containerName='AnalysisMuons')
    subConfig.setOptionValue ('.trackSelection', False)
    configSeq += subConfig
    subConfig = makeConfig ('Muons.WorkingPoint', containerName='AnalysisMuons',
        selectionName='loose')
    subConfig.setOptionValue ('.quality', 'Loose')
    subConfig.setOptionValue ('.isolation', 'NonIso')
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisMuons')
    subConfig.setOptionValue ('.selectionName', 'loose')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    # set up the electron analysis config (For SiHits electrons, use: LooseLHElectronSiHits.NonIso):

    logPLCPAlgCfg.info('Do Electrons')

    subConfig = makeConfig ('Electrons', containerName='AnalysisElectrons')
    subConfig.setOptionValue ('.trackSelection', False)
    subConfig.setOptionValue ('.isolationCorrection', True)
    subConfig.setOptionValue ('.minPt', 0.)
    configSeq += subConfig
    subConfig = makeConfig ('Electrons.WorkingPoint', containerName='AnalysisElectrons',
        selectionName='loose')
    subConfig.setOptionValue ('.likelihoodWP', 'LooseLHElectron')
    subConfig.setOptionValue ('.isolationWP', 'NonIso')
    subConfig.setOptionValue ('.doFSRSelection', True)
    subConfig.setOptionValue ('.noEffSF', True)
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisElectrons')
    subConfig.setOptionValue ('.selectionName', 'loose')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    # So SiHit electrons - should come after the standard selection in order to avoid keeping the same electrons twice
    subConfig = makeConfig ('Electrons', containerName='AnalysisSiHitElectrons')
    subConfig.setOptionValue ('.trackSelection', False)
    subConfig.setOptionValue ('.isolationCorrection', True)
    subConfig.setOptionValue ('.minPt', 0.)
    subConfig.setOptionValue ('.postfix', 'SiHit')
    configSeq += subConfig
    subConfig = makeConfig ('Electrons.WorkingPoint', containerName='AnalysisSiHitElectrons', selectionName='SiHits')
    subConfig.setOptionValue ('.likelihoodWP', 'SiHitElectron')
    subConfig.setOptionValue ('.isolationWP', 'NonIso')
    subConfig.setOptionValue ('.doFSRSelection', True) # needed to veto FSR electrons 
    subConfig.setOptionValue ('.noEffSF', True)
    subConfig.setOptionValue ('.postfix', 'SiHit')
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisSiHitElectrons')
    subConfig.setOptionValue ('.selectionName', 'SiHits')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    # set up the photon analysis config:                                       

    logPLCPAlgCfg.info('Do Photons')

    subConfig = makeConfig ('Photons', containerName='AnalysisPhotons')
    subConfig.setOptionValue ('.recomputeIsEM', False)
    subConfig.setOptionValue ('.minPt', 0.)
    configSeq += subConfig
    subConfig = makeConfig ('Photons.WorkingPoint', containerName='AnalysisPhotons',
        selectionName='loose')
    subConfig.setOptionValue ('.qualityWP', 'Loose')
    subConfig.setOptionValue ('.isolationWP', 'NonIso')
    subConfig.setOptionValue ('.doFSRSelection', True)
    subConfig.setOptionValue ('.recomputeIsEM', False)
    subConfig.setOptionValue ('.noEffSF', True)
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisPhotons')
    subConfig.setOptionValue ('.selectionName', 'loose')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig



    # set up the tau analysis algorithm config:
    # Commented for now due to use of public tools
    subConfig = makeConfig ('TauJets', containerName='AnalysisTauJets')
    configSeq += subConfig
    subConfig = makeConfig ('TauJets.WorkingPoint', containerName='AnalysisTauJets',
        selectionName='baseline')
    subConfig.setOptionValue ('.quality', 'Baseline')
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisTauJets')
    subConfig.setOptionValue ('.selectionName', 'baseline')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    # set up the jet analysis algorithm config:
    jetContainer = 'AntiKt4EMPFlowJets'
    subConfig = makeConfig ('Jets', containerName='AnalysisJets',
        jetCollection=jetContainer)
    subConfig.setOptionValue ('.runFJvtUpdate', False)
    subConfig.setOptionValue ('.runFJvtSelection', False)
    subConfig.setOptionValue ('.runJvtSelection', False)
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisJets')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    largeRjetContainer='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets'
    subConfig = makeConfig ('Jets', containerName='AnalysisLargeRJets',
        jetCollection=largeRjetContainer)
    subConfig.setOptionValue ('.runGhostMuonAssociation', False)
    subConfig.setOptionValue ('.postfix', 'largeR_jets' )
    configSeq += subConfig
    subConfig = makeConfig ('Thinning', containerName='AnalysisLargeRJets')
    subConfig.setOptionValue ('.deepCopy', True)
    subConfig.setOptionValue ('.noUniformSelection', True)
    configSeq += subConfig

    from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
    configAccumulator = ConfigAccumulator (dataType=None, algSeq=None,
        autoconfigFromFlags=flags, noSysSuffix=True)
    configSeq.fullConfigure (configAccumulator)
    return configAccumulator.CA



# Main algorithm config
def PHYSLITEKernelCfg(flags, name='PHYSLITEKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYSLITE"""
    acc = ComponentAccumulator()

    # This block does the common physics augmentation  which isn't needed (or possible) for PHYS->PHYSLITE
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
        'ElectronCaloClusterThinningToolName' : "PHYSLITEElectronCaloClusterThinningTool",
        'PhotonCaloClusterThinningToolName'   : "PHYSLITEPhotonCaloClusterThinningTool",     
        'ElectronGSFTPThinningToolName'       : "PHYSLITEElectronGSFTPThinningTool",
        'PhotonGSFTPThinningToolName'         : "PHYSLITEPhotonGSFTPThinningTool"
    }
    # whereas these are only needed if the input is AOD since they are applied already in PHYS
    if 'StreamAOD' in flags.Input.ProcessingTags:
        thinningToolsArgs.update({
            'TrackParticleThinningToolName'       : "PHYSLITETrackParticleThinningTool",
            'MuonTPThinningToolName'              : "PHYSLITEMuonTPThinningTool",
            'TauJetThinningToolName'              : "PHYSLITETauJetThinningTool",
            'TauJets_MuonRMThinningToolName'      : "PHYSLITETauJets_MuonRMThinningTool",
            'DiTauTPThinningToolName'             : "PHYSLITEDiTauTPThinningTool",
            'DiTauLowPtThinningToolName'          : "PHYSLITEDiTauLowPtThinningTool",
            'DiTauLowPtTPThinningToolName'        : "PHYSLITEDiTauLowPtTPThinningTool",
        })
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))


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

    # Needed in principle to support MET association when running PHYS->PHYSLITE, 
    # but since this doesn't work for PHYS->PHYSLITE anyway, commenting for now
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
        PHYSLITE_cfg = METAssocConfig('AnalysisMET',
                                      flags,
                                      associators,
                                      doPFlow=True,
                                      usePFOLinks=True)
        components_PHYSLITE_cfg = getAssocCA(PHYSLITE_cfg,METName='AnalysisMET')
        acc.merge(components_PHYSLITE_cfg)
    elif 'StreamDAOD_PHYS' in flags.Input.ProcessingTags:
        from DerivationFrameworkJetEtMiss.METCommonConfig import METRemappingCfg

        METRemap_cfg = METRemappingCfg(flags)
        acc.merge(METRemap_cfg)

    # The derivation kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools)) 

    return acc


def PHYSLITECfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSLITETriggerListsHelper = TriggerListsHelper(flags)

    # Set the stream name - varies depending on whether the input is AOD or DAOD_PHYS
    streamName = 'StreamDAOD_PHYSLITE' if 'StreamAOD' in flags.Input.ProcessingTags else 'StreamD2AOD_PHYSLITE' 

    # Common augmentations
    acc.merge(PHYSLITEKernelCfg(flags, name="PHYSLITEKernel", StreamName = streamName, TriggerListsHelper = PHYSLITETriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    PHYSLITESlimmingHelper = SlimmingHelper("PHYSLITESlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, ConfigFlags = flags)

    # Trigger content
    PHYSLITESlimmingHelper.IncludeTriggerNavigation = False
    PHYSLITESlimmingHelper.IncludeJetTriggerContent = False
    PHYSLITESlimmingHelper.IncludeMuonTriggerContent = False
    PHYSLITESlimmingHelper.IncludeEGammaTriggerContent = False
    PHYSLITESlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    PHYSLITESlimmingHelper.IncludeTauTriggerContent = False
    PHYSLITESlimmingHelper.IncludeEtMissTriggerContent = False
    PHYSLITESlimmingHelper.IncludeBJetTriggerContent = False
    PHYSLITESlimmingHelper.IncludeBPhysTriggerContent = False
    PHYSLITESlimmingHelper.IncludeMinBiasTriggerContent = False
    
    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        # Need to re-run matching so that new Analysis<X> containers are matched to triggers
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        acc.merge(TriggerMatchingCommonRun2Cfg(flags, 
                                               name = "PHYSLITETrigMatchNoTau", 
                                               OutputContainerPrefix = "AnalysisTrigMatch_", 
                                               ChainNames = PHYSLITETriggerListsHelper.Run2TriggerNamesNoTau,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        acc.merge(TriggerMatchingCommonRun2Cfg(flags, 
                                               name = "PHYSLITETrigMatchTau", 
                                               OutputContainerPrefix = "AnlaysisTrigMatch_", 
                                               ChainNames = PHYSLITETriggerListsHelper.Run2TriggerNamesTau, 
                                               DRThreshold = 0.2,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        # Now add the resulting decorations to the output 
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSLITESlimmingHelper, 
                                         OutputContainerPrefix = "AnalysisTrigMatch_", 
                                         TriggerList = PHYSLITETriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSLITESlimmingHelper, 
                                         OutputContainerPrefix = "AnalysisTrigMatch_",
                                         TriggerList = PHYSLITETriggerListsHelper.Run2TriggerNamesNoTau)

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        # No need to run matching: just keep navigation so matching can be done by analysts
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSLITESlimmingHelper)

    # Event content
    PHYSLITESlimmingHelper.AppendToDictionary.update({
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
        'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer',
        'HardScatterVertices':'xAOD::TruthVertexContainer','HardScatterVerticesAux':'xAOD::TruthVertexAuxContainer',
        'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
        'AnalysisElectrons':'xAOD::ElectronContainer', 'AnalysisElectronsAux':'xAOD::ElectronAuxContainer',
        'AnalysisSiHitElectrons':'xAOD::ElectronContainer', 'AnalysisSiHitElectronsAux':'xAOD::ElectronAuxContainer',
        'AnalysisMuons':'xAOD::MuonContainer', 'AnalysisMuonsAux':'xAOD::MuonAuxContainer',
        'AnalysisJets':'xAOD::JetContainer','AnalysisJetsAux':'xAOD::AuxContainerBase',
        'AnalysisPhotons':'xAOD::PhotonContainer', 'AnalysisPhotonsAux':'xAOD::PhotonAuxContainer',
        'AnalysisTauJets':'xAOD::TauJetContainer', 'AnalysisTauJetsAux':'xAOD::TauJetAuxContainer',
        'MET_Core_AnalysisMET':'xAOD::MissingETContainer', 'MET_Core_AnalysisMETAux':'xAOD::MissingETAuxContainer',
        'METAssoc_AnalysisMET':'xAOD::MissingETAssociationMap', 'METAssoc_AnalysisMETAux':'xAOD::MissingETAuxAssociationMap',
        'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
        'AnalysisLargeRJets':'xAOD::JetContainer','AnalysisLargeRJetsAux':'xAOD::AuxContainerBase'
    })

    PHYSLITESlimmingHelper.SmartCollections = [
        'EventInfo',
        'InDetTrackParticles',
        'PrimaryVertices',
    ]
    
 
   
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonVariablesCfg    
    PHYSLITESlimmingHelper.ExtraVariables = [ 
        'AnalysisElectrons.trackParticleLinks.f1.pt.eta.phi.m.charge.author.DFCommonElectronsLHVeryLoose.DFCommonElectronsLHLoose.DFCommonElectronsLHLooseBL.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsLHVeryLooseIsEMValue.DFCommonElectronsLHLooseIsEMValue.DFCommonElectronsLHLooseBLIsEMValue.DFCommonElectronsLHMediumIsEMValue.DFCommonElectronsLHTightIsEMValue.DFCommonElectronsECIDS.DFCommonElectronsECIDSResult.topoetcone20.topoetcone20ptCorrection.neflowisol20.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.topoetcone20_CloseByCorr.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.caloClusterLinks.ambiguityLink.TruthLink.truthParticleLink.truthOrigin.truthType.truthPdgId.firstEgMotherTruthType.firstEgMotherTruthOrigin.firstEgMotherTruthParticleLink.firstEgMotherPdgId.ambiguityType.OQ',
        'AnalysisSiHitElectrons.pt.eta.phi.m.charge.author.topoetcone20_CloseByCorr.DFCommonElectronsLHVeryLoose.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr.OQ.truthOrigin.truthType.firstEgMotherTruthType.firstEgMotherTruthOrigin.z0stheta.d0Normalized.nInnerExpPix.clEta.clPhi',
        'AnalysisPhotons.f1.pt.eta.phi.m.author.OQ.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMTight.DFCommonPhotonsIsEMTightIsEMValue.DFCommonPhotonsCleaning.DFCommonPhotonsCleaningNoTime.ptcone20.topoetcone20.topoetcone40.topoetcone20ptCorrection.topoetcone40ptCorrection.topoetcone20_CloseByCorr.topoetcone40_CloseByCorr.ptcone20_CloseByCorr.caloClusterLinks.vertexLinks.ambiguityLink.TruthLink.truthParticleLink.truthOrigin.truthType',
        'GSFTrackParticles.chiSquared.phi.d0.theta.qOverP.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.z0.vz.charge.vertexLink.numberOfPixelHits.numberOfSCTHits.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberOfInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerHits.originalTrackParticle',
        'GSFConversionVertices.trackParticleLinks.x.y.z.px.py.pz.pt1.pt2.neutralParticleLinks.minRfirstHit',
        'egammaClusters.calE.calEta.calPhi.calM.e_sampl.eta_sampl.ETACALOFRAME.PHICALOFRAME.ETA2CALOFRAME.PHI2CALOFRAME.constituentClusterLinks',
        "AnalysisMuons.{var_string}".format(var_string = ".".join(MuonVariablesCfg(flags))),
        'CombinedMuonTrackParticles.qOverP.d0.z0.vz.phi.theta.truthOrigin.truthType.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfTRTHits.numberOfTRTOutliers.chiSquared.numberDoF',
        'ExtrapolatedMuonTrackParticles.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.truthOrigin.truthType.qOverP.theta.phi',
        'MuonSpectrometerTrackParticles.phi.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.vertexLink.theta.qOverP.truthParticleLink',
        'AnalysisTauJets.pt.eta.phi.m.ptFinalCalib.etaFinalCalib.ptTauEnergyScale.etaTauEnergyScale.charge.isTauFlags.PanTau_DecayMode.NNDecayMode.RNNJetScore.RNNJetScoreSigTrans.JetDeepSetScore.JetDeepSetScoreTrans.JetDeepSetVeryLoose.JetDeepSetLoose.JetDeepSetMedium.JetDeepSetTight.RNNEleScore.RNNEleScoreSigTrans_v1.EleRNNLoose_v1.EleRNNMedium_v1.EleRNNTight_v1.tauTrackLinks.vertexLink.truthParticleLink.truthJetLink.IsTruthMatched.truthOrigin.truthType',
        'AnalysisJets.pt.eta.phi.m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.NumTrkPt500.SumPtTrkPt500.DetectorEta.JVFCorr.NNJvtPass.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.PartonTruthLabelID.ConeTruthLabelID.HadronConeExclExtendedTruthLabelID.HadronConeExclTruthLabelID.TrueFlavor.DFCommonJets_jetClean_LooseBad.DFCommonJets_jetClean_TightBad.Timing.btagging.btaggingLink.GhostTrack.DFCommonJets_fJvt.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PSFrac.JetAccessorMap.EMFrac.Width.ActiveArea4vec_pt.ActiveArea4vec_eta.ActiveArea4vec_m.ActiveArea4vec_phi.EnergyPerSampling.SumPtChargedPFOPt500.isJvtHS',
        'BTagging_AntiKt4EMPFlow.DL1dv01_pu.DL1dv01_pc.DL1dv01_pb.GN2v00_pu.GN2v00_pc.GN2v00_pb',
        'AntiKt10UFOCSSKJets.GhostAntiKtVR30Rmax4Rmin02PV0TrackJets',
        'BTagging_AntiKtVR30Rmax4Rmin02Track.DL1dv01_pu.DL1dv01_pc.DL1dv01_pb.GN2v00_pu.GN2v00_pc.GN2v00_pb',
        'TruthPrimaryVertices.t.x.y.z',
        'MET_Core_AnalysisMET.name.mpx.mpy.sumet.source',
        'METAssoc_AnalysisMET.',
        'InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.numberOfTRTHits.numberOfTRTOutliers',
        'EventInfo.RandomRunNumber.PileupWeight_NOSYS.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ',
        'Kt4EMPFlowEventShape.Density',
        'TauTracks.pt.eta.phi.flagSet.trackLinks',
        'AnalysisLargeRJets.pt.eta.phi.m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.DetectorEta.TrackSumMass.TrackSumPt.constituentLinks.ECF1.ECF2.ECF3.Tau1_wta.Tau2_wta.Tau3_wta.Split12.Split23.Qw.D2.C2.R10TruthLabel_R22v1.R10TruthLabel_R21Precision_2022v1.R10TruthLabel_R21Precision.GhostBHadronsFinalCount.GhostCHadronsFinalCount.Parent.GN2Xv01_phbb.GN2Xv01_phcc.GN2Xv01_ptop.GN2Xv01_pqcd'
    ]

    # Truth extra content

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSLITESlimmingHelper)

    # add in extra values for Higgs 
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(flags, PHYSLITESlimmingHelper)

    # Output stream    
    PHYSLITEItemList = PHYSLITESlimmingHelper.GetItemList()
    formatString = 'D2AOD_PHYSLITE' if 'StreamDAOD_PHYS' in flags.Input.ProcessingTags else 'DAOD_PHYSLITE'
    acc.merge(OutputStreamCfg(flags, formatString, ItemList=PHYSLITEItemList, AcceptAlgs=["PHYSLITEKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, formatString, AcceptAlgs=["PHYSLITEKernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

