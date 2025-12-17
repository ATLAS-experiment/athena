# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# HION15.py 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
#Skiming
def HION15SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    JetColl = flags.HeavyIon.HIJetPrefix
    ExtraData  = []
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt2HIJets']
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt4HIJets']

    acc.addSequence( seqAND("HION15Sequence") )
    acc.getSequence("HION15Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION15Sequence").ProcessDynamicDataDependencies = True
    
    expression = ""
    #Trigger selection
    from DerivationFrameworkHI import ListTriggers
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isSmallSystem = False
    if (info.getBeam1Type() < 11) or (info.getBeam2Type() < 11):
        isSmallSystem = True
    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        print('project: ', flags.Input.ProjectName,', isSmallSystem: ', isSmallSystem)
        TriggerDict = ListTriggers.GetTriggers(flags.Input.ProjectName, isSmallSystem)
        for i, key in enumerate(TriggerDict):
            expression = expression + '(' + key + ' && count('+JetColl+'AntiKt4HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) ' + '|| (' + key + ' && count('+JetColl+'AntiKt2HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) '
            if not i == len(TriggerDict) - 1:
                expression = expression + ' || '
    else:
        expression = expression + 'count('+JetColl+'AntiKt2HIJets.pt > 15000) > 1 || count('+JetColl+'AntiKt4HIJets.pt > 15000) > 1'

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION15StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                      primary = True)

    return(acc)                             

def HION15KernelCfg(flags, name='HION15Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    from DerivationFrameworkHI.HION7 import PhysAugmentationsHION7Cfg
    acc.merge(PhysAugmentationsHION7Cfg(flags))
    thinningTools = []
    skimmingTool = acc.getPrimaryAndMerge(HION15SkimmingToolCfg(flags))
    from DerivationFrameworkHI.HION7 import HION7GlobalAugmentationToolCfg
    globalAugmentationTool = acc.getPrimaryAndMerge(HION7GlobalAugmentationToolCfg(flags))
    augmentationTool=[globalAugmentationTool]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,ThinningTools = thinningTools, SkimmingTools = [skimmingTool], AugmentationTools=augmentationTool),sequenceName="HION15Sequence")

    return acc


def HION15Cfg(flags):
    
    acc = ComponentAccumulator()

    JetColl = flags.HeavyIon.HIJetPrefix
    from DerivationFrameworkHI.HION7 import getDFJets
    acc.merge(getDFJets(flags))

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
            FtagBaseContent.add_truth_to_SlimmingHelper(HION15SlimmingHelper)
    if flags.HeavyIon.doHIBTagging:
        from DerivationFrameworkFlavourTag.FtagBaseContent import addCommonAugmentation
        addCommonAugmentation(flags, acc, HION15SlimmingHelper, JetColl+"AntiKt4HIJets")
        AllVars += ListSlimming.HION15AllVarFromFTAG1()
        # update AppendToDictionary
        extra_AppendToDictionary = {}
        FtagBaseContent.update_AppendToDictionary_in_SlimmingHelper(HION15SlimmingHelper, flags, extra_AppendToDictionary)
        # Add ExtraVariables
        ExtraVars += ListSlimming.HION15ExtraVarForBtag(JetColl)
        FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(HION15SlimmingHelper, flags)

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
