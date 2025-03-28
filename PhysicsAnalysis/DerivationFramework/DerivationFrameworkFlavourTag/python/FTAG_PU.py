# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG_PU.py
# This defines DAOD_FTAG_PU, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG_PU in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    addCommonAugmentation
)

# Main algorithm config
def FTAG_PUKernelCfg(flags, name='FTAG_PUKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG_PU"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    skimmingTools = []
    thinningTools = []
    '''
    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import JetTrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg

    
    # filter leptons
    # 2-leptons
    lepton_skimming_expression = 'count( (Muons.pt > 18*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 18*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 2 && count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'
    # 1-lepton + 1-tau
    taul_skimming_expression = '(count( TauJets.pt >= 20*GeV && abs(TauJets.eta) < 2.5 && abs(TauJets.charge)==1.0 && (TauJets.nTracks == 1 || TauJets.nTracks == 3) && TauJets.DFTauLoose) >= 1) && (count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1)'

    total_skimming_expression = '('+lepton_skimming_expression+') || ('+taul_skimming_expression+')'
    
    FTAG_PULeptonSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG_PULeptonSkimmingTool",
            expression = total_skimming_expression )
    acc.addPublicTool(FTAG_PULeptonSkimmingTool)


    # TrackParticles associated with small-R jets
    FTAG_PUAkt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
        name            = "FTAG_PUAkt4PFlowJetTPThinningTool",
        StreamName      = kwargs['StreamName'],
        JetKey   = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    FTAG_PUMuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "FTAG_PUMuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with electrons
    FTAG_PUElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "FTAG_PUElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                 = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [
            FTAG_PUMuonTPThinningTool,
            FTAG_PUElectronTPThinningTool,
            FTAG_PUAkt4PFlowJetTPThinningTool,
            ]
    skimmingTools = [
            FTAG_PULeptonSkimmingTool,
            ]
    '''
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))       
    
    # Extra jet content:
    acc.merge(FTAG_PUExtraContentCfg(flags))
    
    return acc


def FTAG_PUCfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG_PUTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(FTAG_PUKernelCfg(flags, name="FTAG_PUKernel", StreamName = 'StreamDAOD_FTAG_PU', TriggerListsHelper = FTAG_PUTriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG_PUSlimmingHelper = SlimmingHelper("FTAG_PUSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    from DerivationFrameworkFlavourTag import FtagBaseContent

    addCommonAugmentation(flags, acc, FTAG_PUSlimmingHelper)

    FTAG_PUSlimmingHelper.SmartCollections = ["AntiKt4EMPFlowJets",
                                            "AntiKt4TruthJets",
                                            ]
    #FtagBaseContent.add_baseline_slimming_smartcollections(FTAG_PUSlimmingHelper)
    
    FTAG_PUSlimmingHelper.AllVariables = ["EventInfo",
                                        "PrimaryVertices",
                                        "InDetTrackParticles",
                                        ]
    
    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG_PUSlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

    # Add ExtraVariables
    #FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG_PUSlimmingHelper, flags)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG_PUSlimmingHelper, 'FTAG_PU')
    FtagBaseContent.trigger_matching(FTAG_PUSlimmingHelper, FTAG_PUTriggerListsHelper, flags)

    jetOutputList = ["AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(FTAG_PUSlimmingHelper, jetOutputList, FTAG_PUSlimmingHelper.SmartCollections)

    # Flavour tagging 
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowByVertexJets"], ByVertex=True, dzCut_vec=[5, 4], useMinZ0Vertex_vec=[True,False]))
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowJets"]))

    # Output stream
    FTAG_PUItemList = FTAG_PUSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_FTAG_PU", ItemList=FTAG_PUItemList, AcceptAlgs=["FTAG_PUKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_FTAG_PU", AcceptAlgs=["FTAG_PUKernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def FTAG_PUExtraContentCfg(flags):
    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    jetList = []
    #=======================================
    # CSSK R = 0.4 UFO jets
    #=======================================
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
    jetList += [AntiKt4UFOCSSK]

    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlowByVertex 

    #======================================= 
    # R = 0.4 track-jets (needed for Rtrk) 
    #=======================================
    jetList += [AntiKt4PV0Track]

    #======================================= 
    # R = 0.4 by-vertex jets 
    #=======================================
    jetList += [AntiKt4EMPFlowByVertex]

    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags, generations=4, rejectHadronChildren=True))

    #=======================================
    # Add Run-2 jet trigger collections
    # Only needed for Run-2 due to different aux container type (JetTrigAuxContainer) which required special wrapper for conversion to AuxContainerBase
    # In Run-3, the aux. container type is directly JetAuxContainer (no conversion needed)
    #=======================================

    if flags.Trigger.EDMVersion == 2:
        triggerNames = ["JetContainer_a4tcemsubjesFS", "JetContainer_a4tcemsubjesISFS", "JetContainer_a10tclcwsubjesFS", "JetContainer_GSCJet"]

        for trigger in triggerNames:
            wrapperName = trigger+'AuxWrapper'
            auxContainerName = 'HLT_xAOD__'+trigger+'Aux'

            acc.addEventAlgo(CompFactory.xAODMaker.AuxStoreWrapper( wrapperName, SGKeys = [ auxContainerName+"." ] ))

    return acc