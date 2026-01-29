# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# TauTruthCommonConfig.py
# Schedules all tools needed for tau truth object selection and writes
# results into SG. These may then be accessed along the train.
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def DFCommonTauTruthMatchingToolCfg(flags, name = "DFCommonTauTruthMatchingTool", **kwargs):
    kwargs.setdefault("TruthJetContainerName", "AntiKt4TruthDressedWZJets")
    from TauAnalysisTools.TauAnalysisToolsConfig import TauTruthMatchingToolCfg
    return TauTruthMatchingToolCfg(flags, name, **kwargs)


def TauTruthMatchingWrapperCfg(flags, cont, **kwargs):
    """Configure the tau truth matching wrapper"""
    acc = ComponentAccumulator()
    name = "DFCommon"+cont+"TruthMatchingWrapper"
    DFCommonTauTruthMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(
        DFCommonTauTruthMatchingToolCfg(flags)))
    kwargs.setdefault("TauTruthMatchingTool", DFCommonTauTruthMatchingTool)
    kwargs.setdefault("TauContainerName", cont)
    acc.setPrivateTools(
        CompFactory.DerivationFramework.TauTruthMatchingWrapper(name = name, **kwargs))
    return acc


def TauTruthToolsCfg(flags):
    """Configure tau truth making and matching"""

    acc = ComponentAccumulator()

    # Ensure that we are running on MC
    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        # FIXME If this happens it indicates an issue with the configuration in the caller, so better to throw an exception?!
        return acc

    # truth tau building
    from TauAnalysisTools.TauAnalysisToolsConfig import BuildTruthTausAlgCfg
    acc.merge(BuildTruthTausAlgCfg(flags))

    # tau truth matching, if reconstructed taus are present in the input
    # this should be dropped from derivations and deferred to analysis level (the only use case in derivations is PHYSLITE)
    TauTruthAugmentationTools2 = []
    for cont in ["TauJets","TauJets_EleRM"]:
        if "xAOD::TauJetContainer#"+cont in flags.Input.TypedCollections:
            TauTruthAugmentationTools2.append(
                acc.addPublicTool(acc.popToolsAndMerge(TauTruthMatchingWrapperCfg(flags, cont))))

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation( "TauTruthCommonKernel2",
                                         AugmentationTools = TauTruthAugmentationTools2,
                                         ExtraInputs = {( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+TruthTaus' )} ))

    return acc
