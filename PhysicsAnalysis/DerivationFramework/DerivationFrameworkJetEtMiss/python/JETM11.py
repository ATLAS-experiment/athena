4# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM11.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM11TriggerSkimmingToolCfg(flags):
    """Configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    singleElTriggers = TriggerLists.single_el_Trig(flags)
    singleMuTriggers = TriggerLists.single_mu_Trig(flags)

    JETM11TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "JETM11TrigSkimmingTool1",
                                                                                  TriggerListOR          = singleElTriggers + singleMuTriggers)

    acc.addPublicTool(JETM11TrigSkimmingTool, primary=True)

    return acc

def JETM11StringSkimmingToolCfg(flags):
    """Configure the string skimming tool"""

    acc = ComponentAccumulator()

    cutExpression = "(count(Electrons.DFCommonElectronsLHLoose && Electrons.pt > (24 * GeV) && abs(Electrons.eta) < 2.47) + count(Muons.DFCommonMuonPassPreselection && Muons.pt > (24*GeV) && abs(Muons.eta) < 2.47) ) >= 1"

    JETM11StringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM11StringSkimmingTool",
                                                                                      expression = cutExpression)

    acc.addPublicTool(JETM11StringSkimmingTool, primary=True)

    return(acc)


# Main algorithm config
def JETM11KernelCfg(flags, name='JETM11Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM11"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTools = []
    skimmingTools.append(acc.getPrimaryAndMerge(JETM11TriggerSkimmingToolCfg(flags)))
    skimmingTools.append(acc.getPrimaryAndMerge(JETM11StringSkimmingToolCfg(flags)))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Derivation kernel:
    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import TightMETTriggerDerivationKernelCfg
    acc.merge(TightMETTriggerDerivationKernelCfg(flags, name="JETM11Kernel", skimmingTools = skimmingTools, StreamName = 'StreamDAOD_JETM11'))

    return acc


def JETM11Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM11TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation
    acc.merge(JETM11KernelCfg(flags, name="JETM11Kernel", StreamName = 'StreamDAOD_JETM11', TriggerListsHelper = JETM11TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM11SlimmingHelper = SlimmingHelper("JETM11SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import addMETTriggerDerivationContent
    addMETTriggerDerivationContent(JETM11SlimmingHelper, isLoose=False)

    # Trigger content
    JETM11SlimmingHelper.IncludeTriggerNavigation = False
    JETM11SlimmingHelper.IncludeJetTriggerContent = False
    JETM11SlimmingHelper.IncludeMuonTriggerContent = False
    JETM11SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM11SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM11SlimmingHelper.IncludeTauTriggerContent = False
    JETM11SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM11SlimmingHelper.IncludeBJetTriggerContent = False
    JETM11SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM11SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM11ItemList = JETM11SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM11", ItemList=JETM11ItemList, AcceptAlgs=["JETM11Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM11", AcceptAlgs=["JETM11Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

