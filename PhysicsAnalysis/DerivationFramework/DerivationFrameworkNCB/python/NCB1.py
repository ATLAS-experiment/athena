# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_NCB1.py
# This defines DAOD_NCB1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for cosmic ray and BIB studies
# done by the NCB group
# It requires the flag NCB1 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def NCB1KernelCfg(flags, name='NCB1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for NCB1"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkNCB.NCBCommonConfig import NCBCommonAugmentationsCfg
    acc.merge(NCBCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name))   

    return acc


def NCB1Cfg(flags):

    from AthenaCommon.Logging import logging
    logNCB1 = logging.getLogger('NCB1')
    logNCB1.info('****************** STARTING NCB1 *****************')

    acc = ComponentAccumulator()

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    NCB1TriggerListsHelper = TriggerListsHelper(flags)

    acc.merge(NCB1KernelCfg(flags, name='NCB1Kernel', StreamName = 'StreamDAOD_NCB1', TriggerListsHelper = NCB1TriggerListsHelper))
    
        
    # ============================
    # Define contents of the format
    # =============================
    
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
       
    NCB1SlimmingHelper = SlimmingHelper("NCB1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    NCB1SlimmingHelper.SmartCollections = [
        "MET_Baseline_AntiKt4EMTopo",
        "Electrons",
        "Photons"
    ]    

    NCB1SlimmingHelper.AllVariables = [
        "EventInfo",
        "InDetTrackParticles",
        "CombinedMuonTrackParticles",
        "AntiKt4EMTopoJets",
        "CaloCalTopoClusters",
        "Muons",
        "MuonSegments",
        "MuonSpectrometerTrackParticles",
        "ExtrapolatedMuonTrackParticles",
        "NCB_MuonSegments",
    ]

       
    # Truth extra content
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(NCB1SlimmingHelper)
        NCB1SlimmingHelper.AllVariables += [
            "TruthEvents",
            "TruthVertices",
            "TruthParticles",
            "MET_Truth"
        ]
   
    # Trigger content
    NCB1SlimmingHelper.IncludeTriggerNavigation = False
    NCB1SlimmingHelper.IncludeJetTriggerContent = False
    NCB1SlimmingHelper.IncludeMuonTriggerContent = False
    NCB1SlimmingHelper.IncludeEGammaTriggerContent = False
    NCB1SlimmingHelper.IncludeTauTriggerContent = False
    NCB1SlimmingHelper.IncludeEtMissTriggerContent = False
    NCB1SlimmingHelper.IncludeBJetTriggerContent = False
    NCB1SlimmingHelper.IncludeBPhysTriggerContent = False
    NCB1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    NCB1ItemList = NCB1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_NCB1", ItemList=NCB1ItemList, AcceptAlgs=["NCB1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_NCB1", AcceptAlgs=["NCB1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
