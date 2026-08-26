# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# HION8.py 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
def getDFJets(flags):
    """ Create updated version of AntiKt*HIJets"""
    acc = ComponentAccumulator()

    from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
    acc.merge(HIJetRecCfg(flags))

    return acc


#########################################################################################
#Skiming
def HION8SkimmingToolCfg(flags, format="HION8"):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    JetColl = flags.HeavyIon.HIJetPrefix
    ExtraData  = []
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt2HIJets']
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt4HIJets']

    acc.addSequence( seqAND(format+"Sequence") )
    acc.getSequence(format+"Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence(format+"Sequence").ProcessDynamicDataDependencies = True

    #Trigger selection
    filterList = []
    from DerivationFrameworkHI import ListTriggers
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isSmallSystem = False
    if (info.getBeam1Type() < 11) or (info.getBeam2Type() < 11):
        isSmallSystem = True


    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        print('project: ', flags.Input.ProjectName,
              ', isSmallSystem: ', isSmallSystem)
        TriggerDict = ListTriggers.GetTriggers(flags.Input.ProjectName, isSmallSystem,"HION8")
        for key in TriggerDict:
            filterList_trig = []
            expression = (
                'count('+JetColl+'AntiKt4HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1  ||  ' +
                'count('+JetColl+'AntiKt2HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ')

            StringSkimmingTool = acc.addPublicTool(acc.getPrimaryAndMerge(
                xAODStringSkimmingToolCfg(
                    flags, name = format+"StringSkimmingTool_"+key,
                    expression = expression)), primary = True)
            filterList_trig += [StringSkimmingTool]

            TriggerSkimmingTool = (
                CompFactory.DerivationFramework.TriggerSkimmingTool(
                    name = format+"TriggerSkimmingTool_"+key,
                    TriggerListOR = [key]))
            acc.addPublicTool(TriggerSkimmingTool)
            filterList_trig += [TriggerSkimmingTool]

            SkimmingTool_trig  = (
                CompFactory.DerivationFramework.FilterCombinationAND(
                    name=format+"SkimmingTool_trig_"+key,
                    FilterList=filterList_trig))
            acc.addPublicTool(SkimmingTool_trig)
            filterList += [SkimmingTool_trig]

    else:
        expression = ('count('+JetColl+'AntiKt2HIJets.pt > 15000) > 1 || ' +
                      'count('+JetColl+'AntiKt4HIJets.pt > 15000) > 1')
        StringSkimmingTool = acc.addPublicTool(acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(flags, name = format+"StringSkimmingTool",
                                      expression = expression)), primary = True)
        filterList += [StringSkimmingTool]

    SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
        name=format+"SkimmingTool", FilterList=filterList)
    acc.addPublicTool(SkimmingTool, primary = True)

    return(acc)                             


def HION8KernelCfg(flags, name='HION8Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    from DerivationFrameworkHI.HION7 import (
        PhysAugmentationsHION7Cfg, HION7GlobalAugmentationToolCfg)
    acc.merge(PhysAugmentationsHION7Cfg(flags))
    acc.merge(getDFJets(flags))

#########################################################################################
#Thinning
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isSmallSystem = False
    if (info.getBeam1Type() < 11) or (info.getBeam2Type() < 11):
        isSmallSystem = True
    pTCut = 20
    if isSmallSystem:
        pTCut = 15

    JetColl = flags.HeavyIon.HIJetPrefix 

    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg,JetTrackParticleThinningCfg
    
    minTrackPt = 4
    track_thinning_expression  = "InDetTrackParticles.pt > "+str(minTrackPt)+"*GeV"
    TrackParticleThinningTool  = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
         flags,
         name                    = "PHYSTrackParticleThinningTool",
         StreamName              = kwargs['StreamName'], 
         SelectionString         = track_thinning_expression,
         InDetTrackParticlesKey  = "InDetTrackParticles"))

    AntiKt2HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt2HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = JetColl+"AntiKt2HIJets",
         SelectionString         = JetColl+"AntiKt2HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = JetColl+"AntiKt4HIJets",
         SelectionString         = JetColl+"AntiKt4HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    thinningTools = [TrackParticleThinningTool,
                    AntiKt2HIJetsThinningTool,
                    AntiKt4HIJetsThinningTool]
    if flags.Input.isMC or flags.Overlay.DataOverlay:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg
        truth_thinning_expression = "(TruthParticles.status==1) && (TruthParticles.pt > "+str(minTrackPt-0.2)+"*GeV) && (abs(TruthParticles.eta) < 2.7)"
        TruthParticleThinningTool = acc.getPrimaryAndMerge(GenericTruthThinningCfg(flags,
            name="TruthParticleThinningTool",
            StreamName=kwargs['StreamName'],
            ParticleSelectionString = truth_thinning_expression
            )
        )
        thinningTools += [TruthParticleThinningTool]

#########################################################################################

    skimmingTool = acc.getPrimaryAndMerge(HION8SkimmingToolCfg(flags, format="HION8"))
    globalAugmentationTool = acc.getPrimaryAndMerge(HION7GlobalAugmentationToolCfg(flags))
    augmentationTool=[globalAugmentationTool]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,ThinningTools = thinningTools, SkimmingTools = [skimmingTool], AugmentationTools=augmentationTool),sequenceName="HION8Sequence")

    return acc


def HION8Cfg(flags):
    
    acc = ComponentAccumulator()

    JetColl = flags.HeavyIon.HIJetPrefix

    acc.merge(HION8KernelCfg(flags, name="HION8Kernel",StreamName = "StreamDAOD_HION8"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
#########################################################################################
#Slimming
    HION8SlimmingHelper = SlimmingHelper("HION8SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION8SlimmingHelper.SmartCollections = ListSlimming.HION8SmartCollections()
    AllVars = ListSlimming.HION8AllVarContent()
    ExtraVars = ListSlimming.HION8BasicJetVars(JetColl)
    if flags.Input.isMC or flags.Overlay.DataOverlay:
        AllVars += ListSlimming.HION8AllVarTruthContent()

    HION8SlimmingHelper.ExtraVariables = ExtraVars
    HION8SlimmingHelper.AllVariables = AllVars

    HION8ItemList  = HION8SlimmingHelper.GetItemList()
    HIJetRemovedBranches=ListSlimming.makeHIJetRemovedBranchList()
    jet_var_str = '.-'.join ([''] + HIJetRemovedBranches)
    jetRlist = flags.HeavyIon.Jet.RValues #Default [0.2,0.4]
    for jetR in jetRlist:
        output = ["xAOD::JetContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJets",
                "xAOD::JetAuxContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJetsAux.-PseudoJet"+jet_var_str]
        HION8ItemList += output

    acc.merge(OutputStreamCfg(flags, "DAOD_HION8", ItemList=HION8ItemList, AcceptAlgs=["HION8Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION8", AcceptAlgs=["HION8Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

