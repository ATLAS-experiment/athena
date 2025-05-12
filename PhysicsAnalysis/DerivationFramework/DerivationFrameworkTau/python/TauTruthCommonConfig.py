# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# TauTruthCommonConfig.py
# Schedules all tools needed for tau truth object selection and writes
# results into SG. These may then be accessed along the train.
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# TODO: should this be in a separate file or ok here?
def TauTruthMatchingWrapperCfg(flags, name = "DFCommonTauTruthMatchingWrapper", **kwargs):
    """Configure the tau truth matching wrapper"""
    acc = ComponentAccumulator()
    TauTruthMatchingWrapper = CompFactory.DerivationFramework.TauTruthMatchingWrapper
    acc.addPublicTool(TauTruthMatchingWrapper(name = name, **kwargs), primary = True)
    return(acc)

# TODO: should this be in a separate file or ok here?
def TruthCollectionMakerTauCfg(flags, name, **kwargs):
    """Configure the tau truth collection maker"""
    acc = ComponentAccumulator()
    TruthCollectionMakerTau = CompFactory.DerivationFramework.TruthCollectionMakerTau
    acc.addPublicTool(TruthCollectionMakerTau(name, **kwargs), primary = True)
    return(acc)

# TODO: should be a common cfg in TauAnalysisTools
def TauTruthMatchingToolCfg(flags, name="DFCommonTauTruthMatchingTool", **kwargs):
    """Configure the tau truth matching tool"""
    acc = ComponentAccumulator()
    TauTruthMatchingTool = CompFactory.TauAnalysisTools.TauTruthMatchingTool
    acc.addPublicTool(TauTruthMatchingTool(name = name, **kwargs), primary = True)
    return(acc)

# TODO: should be a common cfg in TauAnalysisTools
def BuildTruthTausCfg(flags, name, **kwargs):
    """Configure the BuildTruthTaus tool"""
    acc = ComponentAccumulator()
    BuildTruthTaus = CompFactory.TauAnalysisTools.BuildTruthTaus
    acc.addPublicTool(BuildTruthTaus(name = name, **kwargs), primary = True)
    return(acc)

def TauTruthToolsCfg(flags):
    """Configure tau truth making and matching"""

    # Ensure that we are running on MC
    if not flags.Input.isMC:
        return

    acc = ComponentAccumulator()

    DFCommonTauTruthWrapperTools = []

    # truth tau building
    acc.merge(BuildTruthTausCfg(flags,
                                name                            = "DFCommonTauTruthBuilder",
                                WriteInvisibleFourMomentum      = True,
                                WriteVisibleNeutralFourMomentum = True ))

    acc.merge(TruthCollectionMakerTauCfg(flags,
                                         name           = "DFCommonTauTruthCollectionMaker",
                                         BuildTruthTaus = acc.getPublicTool("DFCommonTauTruthBuilder")))
    DFCommonTauTruthWrapperTools.append(acc.getPublicTool("DFCommonTauTruthCollectionMaker"))

    # tau truth matching, if reconstructed taus are present in the input
    # this should be dropped from derivations and deferred to analysis level (the only use case in derivations is PHYSLITE)
    DFCommonTauTruthMatchingTool = acc.getPrimaryAndMerge(TauTruthMatchingToolCfg(
            flags,
            name                            = "DFCommonTauTruthMatchingTool",
            TruthJetContainerName           = "AntiKt4TruthDressedWZJets"))

    for cont in ["TauJets","TauJets_EleRM"]:
        if "xAOD::TauJetContainer#"+cont in flags.Input.TypedCollections: 
            DFCommonTauTruthWrapperTool = acc.getPrimaryAndMerge(TauTruthMatchingWrapperCfg(
                flags,
                name                 = "DFCommon"+cont+"TruthMatchingWrapper",
                TauTruthMatchingTool = DFCommonTauTruthMatchingTool,
                TauContainerName     = cont))
            DFCommonTauTruthWrapperTools.append(DFCommonTauTruthWrapperTool)

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation( "TauTruthCommonKernel", AugmentationTools = DFCommonTauTruthWrapperTools,
                                        ExtraOutputs = {( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+TruthTaus' ),
                                                        ( 'xAOD::IParticleContainer' , 'StoreGateSvc+TruthTaus' )} ))

    return acc    
