# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddTauAugmentationCfg(flags, **kwargs):

    prefix = kwargs["prefix"]
    kwargs.setdefault("doVeryLoose", False)
    kwargs.setdefault("doLoose",     False)
    kwargs.setdefault("doMedium",    False)
    kwargs.setdefault("doTight",     False)

    acc = ComponentAccumulator()

    # tau selection relies on RNN electron veto, we must decorate the fixed eveto WPs before applying tau selection
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName="TauJets"))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg

    TauAugmentationTools = []

    if kwargs["doVeryLoose"]:
        TauSelectorVeryLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                        name = 'TauSelectorVeryLoose',
                                                                        ConfigPath = 'TauAnalysisAlgorithms/tau_selection_veryloose_noeleid.conf'))
        acc.addPublicTool(TauSelectorVeryLoose)

        TauVeryLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                name               = "TauVeryLooseWrapper",
                                                                                AsgSelectionTool   = TauSelectorVeryLoose,
                                                                                StoreGateEntryName = "DFTauVeryLoose",
                                                                                ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauVeryLooseWrapper)

    if kwargs["doLoose"]:
        TauSelectorLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                    name = 'TauSelectorLoose',
                                                                    ConfigPath = 'TauAnalysisAlgorithms/tau_selection_loose_noeleid.conf'))
        acc.addPublicTool(TauSelectorLoose)

        TauLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                            name               = "TauLooseWrapper",
                                                                            AsgSelectionTool   = TauSelectorLoose,
                                                                            StoreGateEntryName = "DFTauLoose",
                                                                            ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauLooseWrapper)

    if kwargs["doMedium"]:
        TauSelectorMedium = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                     name = 'TauSelectorMedium',
                                                                     ConfigPath = 'TauAnalysisAlgorithms/tau_selection_medium_noeleid.conf'))
        acc.addPublicTool(TauSelectorMedium)

        TauMediumWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                             name               = "TauMediumWrapper",
                                                                             AsgSelectionTool   = TauSelectorMedium,
                                                                             StoreGateEntryName = "DFTauMedium",
                                                                             ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauMediumWrapper)

    if kwargs["doTight"]:
        TauSelectorTight = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                    name = 'TauSelectorTight',
                                                                    ConfigPath = 'TauAnalysisAlgorithms/tau_selection_tight_noeleid.conf'))
        acc.addPublicTool(TauSelectorTight)

        TauTightWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                            name               = "TauTightWrapper",
                                                                            AsgSelectionTool   = TauSelectorTight,
                                                                            StoreGateEntryName = "DFTauTight",
                                                                            ContainerName      = "TauJets"))
        TauAugmentationTools.append(TauTightWrapper)

    if TauAugmentationTools:
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation(f"{prefix}_TauAugmentationKernel", AugmentationTools = TauAugmentationTools))

    return acc


# Low pT di-taus
def AddDiTauLowPtCfg(flags, **kwargs):
    """Configure the low-pt di-tau building"""

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo
    acc.merge(JetRecCfg(flags,AntiKt10LCTopo))

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderLowPtCfg
    acc.merge(DiTauBuilderLowPtCfg(flags, name="DiTauLowPtBuilder"))

    return acc


def AddTauIDDecorationCfg(flags, **kwargs):
    """Decorate tau ID scores and working points"""

    kwargs.setdefault("evetoFix",         True)
    kwargs.setdefault("GNNTauID",         True)
    kwargs.setdefault("TauContainerName", "TauJets")
    kwargs.setdefault("prefix",           kwargs['TauContainerName'])

    acc = ComponentAccumulator()

    import tauRec.TauToolHolder as tauTools
    tools = []

    if kwargs['evetoFix']:
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNFixCfg(flags)) )

    if kwargs['GNNTauID']:    
        # vertex-corrected clusters must be rebuilt for tau ID
        tools.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
        # Add in GNTau!
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,0)) )
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,1)) )
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags,0)) )
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags,1)) )

    if tools:
        for tool in tools:
            acc.addPublicTool(tool)

        TauIDDecoratorWrapper = CompFactory.DerivationFramework.TauIDDecoratorWrapper
        TauIDDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

        prefix = kwargs['prefix']
        tauIDDecoratorWrapper = TauIDDecoratorWrapper(name             = f"{prefix}_TauIDDecoratorWrapper",
                                                      TauContainerName = kwargs['TauContainerName'],
                                                      TauIDTools       = tools)

        acc.addPublicTool(tauIDDecoratorWrapper)
        acc.addEventAlgo(TauIDDecoratorKernel(name              = f"{prefix}_TauIDDecorKernel",
                                              AugmentationTools = [tauIDDecoratorWrapper]))

    return acc


# TauJets_MuonRM steering
def AddMuonRemovalTauAODReRecoAlgCfg(flags, **kwargs):
    """Configure the MuonRM AOD tau building"""

    acc = ComponentAccumulator()

    # get tools from holder
    import tauRec.TauToolHolder as tauTools
    tools_mod = []
    tools_mod.append( acc.popToolsAndMerge(tauTools.TauAODMuonRemovalCfg(flags)) )
    tools_after = []
    tools_after.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauTrackRNNClassifierCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.EnergyCalibrationLCCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauCommonCalcVarsCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauSubstructureCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ClusterCreatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ClusterScalerCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0ScoreCalculatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.Pi0SelectorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauVertexVariablesCfg(flags)) )
    import PanTauAlgs.JobOptions_Main_PanTau as pantau
    tools_after.append( acc.popToolsAndMerge(pantau.PanTauCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauCombinedTESCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.MvaTESVariableDecoratorCfg(flags)) )
    tools_after[-1].EventShapeKey = ''
    tools_after.append( acc.popToolsAndMerge(tauTools.MvaTESEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauIDVarCalculatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauJetRNNEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorJetRNNCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauEleRNNEvaluatorCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauDecayModeNNClassifierCfg(flags)) )
    TauAODRunnerAlg=CompFactory.getComp("TauAODRunnerAlg")
    for tool in tools_mod:
        tool.inAOD = True
    for tool in tools_after:
        tool.inAOD = True
    myTauAODRunnerAlg = TauAODRunnerAlg(  
        name                           = "MuonRemovalTauAODReRecoAlg",
        Key_tauOutputContainer         = "TauJets_MuonRM",
        Key_pi0OutputContainer         = "TauFinalPi0s_MuonRM",
        Key_neutralPFOOutputContainer  = "TauNeutralParticleFlowObjects_MuonRM",
        Key_chargedPFOOutputContainer  = "TauChargedParticleFlowObjects_MuonRM",
        Key_hadronicPFOOutputContainer = "TauHadronicParticleFlowObjects_MuonRM",
        Key_tauTrackOutputContainer    = "TauTracks_MuonRM",
        Key_vertexOutputContainer      = "TauSecondaryVertices_MuonRM",
        modificationTools              = tools_mod,
        officialTools                  = tools_after
    )
    acc.addEventAlgo(myTauAODRunnerAlg)
    return acc


def TauThinningCfg(flags, name, **kwargs):
    """configure tau thinning"""

    acc = ComponentAccumulator()
    TauThinningTool = CompFactory.DerivationFramework.TauThinningTool
    acc.addPublicTool(TauThinningTool(name, **kwargs), primary=True)
    return acc
