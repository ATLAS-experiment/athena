# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM5.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM5SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    expression = '( HLT_noalg_zb_L1ZB || HLT_noalg_L1ZB )'
    JETM5SkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM5SkimmingTool1",
                                                                               expression = expression)

    acc.addPublicTool(JETM5SkimmingTool, primary = True)
        
    return(acc)


# Main algorithm config
def JETM5KernelCfg(flags, name='JETM5Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM5"""
    acc = ComponentAccumulator()

    # Skimming
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(JETM5SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg

    # Include inner detector tracks associated with muons
    JETM5MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM5MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM5ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM5ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    JETM5PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                     = "JETM5PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))


    thinningTools = [JETM5MuonTPThinningTool,
                     JETM5ElectronTPThinningTool,
                     JETM5PhotonTPThinningTool]

    # Truth particle thinning
    if flags.Input.isMC:
        truth_cond_WZH    = "((abs(TruthParticles.pdgId) >= 23) && (abs(TruthParticles.pdgId) <= 25))"                                      # W, Z and Higgs
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && !(TruthParticles.isSimulationParticle))" # Leptons
        truth_cond_Quark  = "((abs(TruthParticles.pdgId) <=  5 && (TruthParticles.pt > 10000.)) || (abs(TruthParticles.pdgId) == 6))"       # Quarks
        truth_cond_Gluon  = "((abs(TruthParticles.pdgId) == 21) && (TruthParticles.pt > 10000.))"                                           # Gluons
        truth_cond_Photon = "((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 10000.) && !(TruthParticles.isSimulationParticle))"      # Photon
        
        truth_expression = '('+truth_cond_WZH+' || '+truth_cond_Lepton +' || '+truth_cond_Quark+'||'+truth_cond_Gluon+' || '+truth_cond_Photon+')'

        preserveAllDescendants = False

        JETM5TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning ( name = "JETM5TruthThinningTool",
                                                                                        StreamName              = kwargs['StreamName'],
                                                                                        ParticleSelectionString = truth_expression,
                                                                                        PreserveDescendants     = preserveAllDescendants,
                                                                                        PreserveGeneratorDescendants = not preserveAllDescendants,
                                                                                        PreserveAncestors = True)

        acc.addPublicTool(JETM5TruthThinningTool)
        thinningTools.append(JETM5TruthThinningTool)
        
    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool] if not flags.Input.isMC else []))       


    # PFlow augmentation tool
    from DerivationFrameworkJetEtMiss.PFlowCommonConfig import PFlowCommonCfg
    acc.merge(PFlowCommonCfg(flags))
    
    return acc


def JETM5Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM5TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM5KernelCfg(flags, name="JETM5Kernel", StreamName = 'StreamDAOD_JETM5', TriggerListsHelper = JETM5TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM5SlimmingHelper = SlimmingHelper("JETM5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM5SlimmingHelper.SmartCollections = ["EventInfo", "InDetTrackParticles", "PrimaryVertices",
                                            "Electrons", "Photons", "Muons", "TauJets",
                                            "MET_Baseline_AntiKt4EMTopo","MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                            "BTagging_AntiKt4EMPFlow"]

    JETM5SlimmingHelper.AllVariables = ["CaloCalTopoClusters",
                                        "MuonSegments",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape",
                                        "GlobalNeutralParticleFlowObjects", "GlobalChargedParticleFlowObjects", 
                                        "CHSGChargedParticleFlowObjects", "CHSGNeutralParticleFlowObjects",
                                        "UFOCSSK"]


    JETM5SlimmingHelper.ExtraVariables  += ["AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1"]

    if flags.Input.isMC:
        JETM5SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer',
                                                       'TruthVertices': 'xAOD::TruthVertexContainer',
                                                       'TruthVerticesAux': 'xAOD::TruthVertexAuxContainer'})

        JETM5SlimmingHelper.AllVariables += ["MuonTruthParticles", "egammaTruthParticles",
                                             "TruthParticles", "TruthEvents", "TruthVertices"]

    # Trigger content
    JETM5SlimmingHelper.IncludeTriggerNavigation = False
    JETM5SlimmingHelper.IncludeJetTriggerContent = False
    JETM5SlimmingHelper.IncludeMuonTriggerContent = False
    JETM5SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM5SlimmingHelper.IncludeTauTriggerContent = False
    JETM5SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM5SlimmingHelper.IncludeBJetTriggerContent = False
    JETM5SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM5SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM5ItemList = JETM5SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM5", ItemList=JETM5ItemList, AcceptAlgs=["JETM5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM5", AcceptAlgs=["JETM5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

