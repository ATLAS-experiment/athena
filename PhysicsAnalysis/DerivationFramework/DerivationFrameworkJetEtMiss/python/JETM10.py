# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM10.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM10SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    JETM10TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "JETM10TrigSkimmingTool1",
                                                                                  TriggerListOR          = ["HLT_noalg_L1XE.*"] )

    acc.addPublicTool(JETM10TrigSkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM10KernelCfg(flags, name='JETM10Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM10"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTool = acc.getPrimaryAndMerge(JETM10SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Derivation kernel:
    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import LooseMETTriggerDerivationKernelCfg
    acc.merge(LooseMETTriggerDerivationKernelCfg(flags, name="JETM10Kernel", skimmingTools = [skimmingTool], StreamName = 'StreamDAOD_JETM10'))

    return acc


def JETM10Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM10TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation
    acc.merge(JETM10KernelCfg(flags, name="JETM10Kernel", StreamName = 'StreamDAOD_JETM10', TriggerListsHelper = JETM10TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM10SlimmingHelper = SlimmingHelper("JETM10SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import addMETTriggerDerivationContent
    addMETTriggerDerivationContent(JETM10SlimmingHelper, isLoose=True)

    # Trigger content
    JETM10SlimmingHelper.IncludeTriggerNavigation = False
    JETM10SlimmingHelper.IncludeJetTriggerContent = False
    JETM10SlimmingHelper.IncludeMuonTriggerContent = False
    JETM10SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM10SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM10SlimmingHelper.IncludeTauTriggerContent = False
    JETM10SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM10SlimmingHelper.IncludeBJetTriggerContent = False
    JETM10SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM10SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM10ItemList = JETM10SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM10", ItemList=JETM10ItemList, AcceptAlgs=["JETM10Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM10", AcceptAlgs=["JETM10Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

