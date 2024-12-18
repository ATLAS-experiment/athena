# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM10.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM10MetTrigSkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    JETM10MetTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name           = "JETM10MetTrigSkimmingTool1",
                                                                                  TriggerListOR     = ["HLT_noalg_L1XE.*","HLT_noalg_L1jXE.*","HLT.*ZeroBias","HLT.*zb.*","HLT.*_eb_.*","HLT.*noPS.*","HLT_noalg_L1RD3.*"] )
    acc.addPublicTool(JETM10MetTrigSkimmingTool, primary = True)

    return(acc)

def JETM10MuTrigSkimmingToolCfg(flags):
    """Configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    singleMuTriggers = TriggerLists.single_mu_Trig(flags)

    JETM10MuTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name         = "JETM10MuTrigSkimmingTool1",
                                                                                  TriggerListOR  = singleMuTriggers)

    acc.addPublicTool(JETM10MuTrigSkimmingTool, primary = True)

    return acc

def JETM10StringSkimmingToolCfg(flags):
    """Configure the string skimming tool"""

    acc = ComponentAccumulator()

    cutExpression = "(count(Muons.DFCommonMuonPassPreselection && Muons.pt > (20*GeV) && abs(Muons.eta) < 2.7) ) >= 2"

    JETM10StringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM10StringSkimmingTool",
                                                                                      expression = cutExpression)

    acc.addPublicTool(JETM10StringSkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM10KernelCfg(flags, name='JETM10Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM10"""
    acc = ComponentAccumulator()

    # Skimming
    JETM10MetTrigSkimmingTool = acc.getPrimaryAndMerge(JETM10MetTrigSkimmingToolCfg(flags))
    JETM10MuTrigSkimmingTool  = acc.getPrimaryAndMerge(JETM10MuTrigSkimmingToolCfg(flags))
    JETM10StringSkimmingTool  = acc.getPrimaryAndMerge(JETM10StringSkimmingToolCfg(flags))

    JETM10MuTrigStringSkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM10MuTrigStringSkimmingTool", FilterList=[JETM10MuTrigSkimmingTool,   JETM10StringSkimmingTool] )
    acc.addPublicTool(JETM10MuTrigStringSkimmingTool)
    JETM10SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(name="JETM10SkimmingTool",
                        FilterList=[JETM10MuTrigStringSkimmingTool, JETM10MetTrigSkimmingTool])
    acc.addPublicTool(JETM10SkimmingTool, primary = True)

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Derivation kernel:
    from DerivationFrameworkJetEtMiss.METTriggerDerivationContentConfig import LooseMETTriggerDerivationKernelCfg
    acc.merge(LooseMETTriggerDerivationKernelCfg(flags, name="JETM10Kernel", skimmingTools = [JETM10SkimmingTool], StreamName = 'StreamDAOD_JETM10'))

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
    JETM10SlimmingHelper.IncludeTauTriggerContent = False
    JETM10SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM10SlimmingHelper.IncludeBJetTriggerContent = False
    JETM10SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM10SlimmingHelper.IncludeMinBiasTriggerContent = False

    JETM10SlimmingHelper.AllVariables += ['HLT_MET_tcpufit','HLT_MET_cell','HLT_MET_trkmht','HLT_MET_cvfpufit','HLT_MET_pfopufit','HLT_MET_mhtpufit_em','HLT_MET_mhtpufit_pf','HLT_MET_pfsum','HLT_MET_pfsum_vssk','HLT_MET_pfsum_cssk','HLT_MET_nn','L1_jFexMETRoI','L1_jFexMETxRoI','L1_gMETComponentsJwoj','L1_gMETComponentsRms','L1_gMETComponentsNoiseCut']

    # Output stream    
    JETM10ItemList = JETM10SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM10", ItemList=JETM10ItemList, AcceptAlgs=["JETM10Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM10", AcceptAlgs=["JETM10Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

