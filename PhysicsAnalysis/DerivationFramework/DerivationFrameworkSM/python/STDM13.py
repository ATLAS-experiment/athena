# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_STDM13.py for W+D analysises
# * 1L skimming
# * InDetTrackParticles with |z0|*sinTheta < 6.0
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def STDM13KernelCfg(flags, name='STDM13Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for STDM13"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    
    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'

    STDM13StringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "STDM13StringSkimmingTool",
            expression = lepton_skimming_expression )
    acc.addPublicTool(STDM13StringSkimmingTool)
    STDM13SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND("STDM13SkimmingTool", FilterList = [STDM13StringSkimmingTool])
    
    acc.addPublicTool(STDM13SkimmingTool, primary = True)
    tp_thinning_expression = "abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 6.0*mm"
    STDM13TPThinningTool =  acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "STDM13TPThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = tp_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    thinningTools = [ STDM13TPThinningTool ]
    skimmingTools = [ STDM13SkimmingTool ]
    
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))       
    return acc


def STDM13Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    STDM13TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(STDM13KernelCfg(flags, name="STDM13Kernel", StreamName = 'StreamDAOD_STDM13', TriggerListsHelper = STDM13TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    STDM13SlimmingHelper = SlimmingHelper("STDM13SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    STDM13SlimmingHelper.SmartCollections = [
        "Electrons",
        "Muons",
        "AntiKt4EMPFlowJets",
        "MET_Baseline_AntiKt4EMPFlow",
        "PrimaryVertices",
        "InDetTrackParticles",
        "BTagging_AntiKt4EMPFlow",
    ]

    STDM13SlimmingHelper.AllVariables = [
        "EventInfo",
        "PrimaryVertices",
        "InDetTrackParticles",
        "BTagging_AntiKt4EMPFlow",
        "AntiKt4TruthDressedWZJets",
        "TruthEvents","TruthHFWithDecayParticles","TruthBoson","TruthBottom", "TruthCharm","TruthElectrons","TruthMuons","TruthTop","TruthTaus","MET_Truth",
        "TruthPrimaryVertices","TruthHFWithDecayVertices",
    ]

    STDM13SlimmingHelper.IncludeTriggerNavigation = False
    STDM13SlimmingHelper.IncludeJetTriggerContent = False
    STDM13SlimmingHelper.IncludeMuonTriggerContent = False
    STDM13SlimmingHelper.IncludeEGammaTriggerContent = False
    STDM13SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    STDM13SlimmingHelper.IncludeTauTriggerContent = False
    STDM13SlimmingHelper.IncludeEtMissTriggerContent = False
    STDM13SlimmingHelper.IncludeBJetTriggerContent = False
    STDM13SlimmingHelper.IncludeBPhysTriggerContent = False
    STDM13SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM13SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = STDM13TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM13SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = STDM13TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(STDM13SlimmingHelper)

    # Output stream    
    STDM13ItemList = STDM13SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_STDM13", ItemList=STDM13ItemList, AcceptAlgs=["STDM13Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_STDM13", AcceptAlgs=["STDM13Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

