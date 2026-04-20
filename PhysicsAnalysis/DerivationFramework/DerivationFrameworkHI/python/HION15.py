# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# HION15.py 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

#########################################################################################

def HION15KernelCfg(flags, name='HION15Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    from DerivationFrameworkHI.HION7 import (
        PhysAugmentationsHION7Cfg, HION7SkimmingToolCfg, HION7GlobalAugmentationToolCfg)
    acc.merge(PhysAugmentationsHION7Cfg(flags))
    from DerivationFrameworkHI.HION7 import getDFJets
    acc.merge(getDFJets(flags))
    thinningTools = []
    skimmingTool = acc.getPrimaryAndMerge(HION7SkimmingToolCfg(flags, format="HION15"))
    globalAugmentationTool = acc.getPrimaryAndMerge(HION7GlobalAugmentationToolCfg(flags))
    augmentationTool=[globalAugmentationTool]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,ThinningTools = thinningTools, SkimmingTools = [skimmingTool], AugmentationTools=augmentationTool),sequenceName="HION15Sequence")

    return acc


def HION15Cfg(flags):
    
    acc = ComponentAccumulator()

    JetColl = flags.HeavyIon.HIJetPrefix

    acc.merge(HION15KernelCfg(flags, name="HION15Kernel",StreamName = "StreamDAOD_HION15"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
#########################################################################################
#Slimming
    HION15SlimmingHelper = SlimmingHelper("HION15SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION15SlimmingHelper.SmartCollections = ListSlimming.HION15SmartCollections()
    AllVars = ListSlimming.HION15AllVarContent()
    AllVars += ListSlimming.HION15ExtraContainersTrigger()
    ExtraVars = ListSlimming.HION15BasicJetVars(JetColl)
    from DerivationFrameworkFlavourTag import FtagBaseContent
    if flags.Input.isMC or flags.Overlay.DataOverlay:
        AllVars += ListSlimming.HION15AllVarTruthContent()
        if flags.HeavyIon.doHIBTagging:
            FtagBaseContent.add_truth_to_slimming_helper(HION15SlimmingHelper)
    if flags.HeavyIon.doHIBTagging:
        from DerivationFrameworkFlavourTag.FtagBaseContent import add_common_augmentation
        add_common_augmentation(flags, acc, HION15SlimmingHelper, JetColl+"AntiKt4HIJets")
        AllVars += ListSlimming.HION15AllVarFromFTAG1()
        # update AppendToDictionary
        extra_AppendToDictionary = {}
        FtagBaseContent.update_append_to_dictionary_in_slimming_helper(flags, HION15SlimmingHelper, extra_AppendToDictionary)
        # Add ExtraVariables
        ExtraVars += ListSlimming.HION15ExtraVarForBtag(JetColl)
        FtagBaseContent.add_extra_variables_to_slimming_helper(flags, HION15SlimmingHelper)

    HION15SlimmingHelper.ExtraVariables = ExtraVars
    HION15SlimmingHelper.AllVariables = AllVars

    HION15ItemList  = HION15SlimmingHelper.GetItemList()
    HIJetRemovedBranches=ListSlimming.makeHIJetRemovedBranchList()
    jet_var_str = '.-'.join ([''] + HIJetRemovedBranches)
    jetRlist = flags.HeavyIon.Jet.RValues #Default [0.2,0.4]
    for jetR in jetRlist:
        output = ["xAOD::JetContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJets",
                "xAOD::JetAuxContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJetsAux.-PseudoJet"+jet_var_str]
        HION15ItemList += output

    acc.merge(OutputStreamCfg(flags, "DAOD_HION15", ItemList=HION15ItemList, AcceptAlgs=["HION15Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION15", AcceptAlgs=["HION15Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

