# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddTauAugmentationCfg(flags, **kwargs):

    prefix = kwargs["prefix"]
    kwargs.setdefault("TauContainerName", "TauJets")
    kwargs.setdefault("doRNNVeryLoose", False)
    kwargs.setdefault("doRNNLoose",     False)
    kwargs.setdefault("doRNNMedium",    False)
    kwargs.setdefault("doRNNTight",     False)
    kwargs.setdefault("doGNTauVeryLoose", False)
    kwargs.setdefault("doGNTauLoose",     False)
    kwargs.setdefault("doGNTauMedium",    False)
    kwargs.setdefault("doGNTauTight",     False)

    acc = ComponentAccumulator()

    # tau selection relies on RNN electron veto, we must decorate the fixed eveto WPs before applying tau selection
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName=kwargs["TauContainerName"]))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg

    TauAugmentationTools = []

    # RNN TauID WPs
    if kwargs["doRNNVeryLoose"]:
        TauSelectorRNNVeryLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                           name = 'TauSelectorRNNVeryLoose',
                                                                           ConfigPath = 'TauAnalysisAlgorithms/tau_selection_veryloose_noeleid.conf'))
        acc.addPublicTool(TauSelectorRNNVeryLoose)

        TauRNNVeryLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                   name               = "TauRNNVeryLooseWrapper",
                                                                                   AsgSelectionTool   = TauSelectorRNNVeryLoose,
                                                                                   StoreGateEntryName = "DFTauRNNVeryLoose",
                                                                                   ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauRNNVeryLooseWrapper)

    if kwargs["doRNNLoose"]:
        TauSelectorRNNLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                       name = 'TauSelectorRNNLoose',
                                                                       ConfigPath = 'TauAnalysisAlgorithms/tau_selection_loose_noeleid.conf'))
        acc.addPublicTool(TauSelectorRNNLoose)

        TauRNNLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                               name               = "TauRNNLooseWrapper",
                                                                               AsgSelectionTool   = TauSelectorRNNLoose,
                                                                               StoreGateEntryName = "DFTauRNNLoose",
                                                                               ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauRNNLooseWrapper)

    if kwargs["doRNNMedium"]:
        TauSelectorRNNMedium = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                        name = 'TauSelectorRNNMedium',
                                                                        ConfigPath = 'TauAnalysisAlgorithms/tau_selection_medium_noeleid.conf'))
        acc.addPublicTool(TauSelectorRNNMedium)

        TauRNNMediumWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                name               = "TauRNNMediumWrapper",
                                                                                AsgSelectionTool   = TauSelectorRNNMedium,
                                                                                StoreGateEntryName = "DFTauRNNMedium",
                                                                                ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauRNNMediumWrapper)

    if kwargs["doRNNTight"]:
        TauSelectorRNNTight = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                    name = 'TauSelectorRNNTight',
                                                                    ConfigPath = 'TauAnalysisAlgorithms/tau_selection_tight_noeleid.conf'))
        acc.addPublicTool(TauSelectorRNNTight)

        TauRNNTightWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                               name               = "TauRNNTightWrapper",
                                                                               AsgSelectionTool   = TauSelectorRNNTight,
                                                                               StoreGateEntryName = "DFTauRNNTight",
                                                                               ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauRNNTightWrapper)

    # GNTau TauID WPs
    if kwargs["doGNTauVeryLoose"]:
        TauSelectorGNTauVeryLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                             name = 'TauSelectorGNTauVeryLoose',
                                                                             ConfigPath = 'TauAnalysisAlgorithms/tau_selection_gntau_veryloose_noeleid.conf'))
        acc.addPublicTool(TauSelectorGNTauVeryLoose)

        TauGNTauVeryLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                     name               = "TauGNTauVeryLooseWrapper",
                                                                                     AsgSelectionTool   = TauSelectorGNTauVeryLoose,
                                                                                     StoreGateEntryName = "DFTauGNTauVeryLoose",
                                                                                     ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauGNTauVeryLooseWrapper)
 
    
    if kwargs["doGNTauLoose"]:
        TauSelectorGNTauLoose = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                         name = 'TauSelectorGNTauLoose',
                                                                         ConfigPath = 'TauAnalysisAlgorithms/tau_selection_gntau_loose_noeleid.conf'))
        acc.addPublicTool(TauSelectorGNTauLoose)

        TauGNTauLooseWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                 name               = "TauGNTauLooseWrapper",
                                                                                 AsgSelectionTool   = TauSelectorGNTauLoose,
                                                                                 StoreGateEntryName = "DFTauGNTauLoose",
                                                                                 ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauGNTauLooseWrapper)

    if kwargs["doGNTauMedium"]:
        TauSelectorGNTauMedium = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                          name = 'TauSelectorGNTauMedium',
                                                                          ConfigPath = 'TauAnalysisAlgorithms/tau_selection_gntau_medium_noeleid.conf'))
        acc.addPublicTool(TauSelectorGNTauMedium)

        TauGNTauMediumWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                  name               = "TauGNTauMediumWrapper",
                                                                                  AsgSelectionTool   = TauSelectorGNTauMedium,
                                                                                  StoreGateEntryName = "DFTauGNTauMedium",
                                                                                  ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauGNTauMediumWrapper)

    if kwargs["doGNTauTight"]:
        TauSelectorGNTauTight = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                         name = 'TauSelectorGNTauTight',
                                                                         ConfigPath = 'TauAnalysisAlgorithms/tau_selection_gntau_tight_noeleid.conf'))
        acc.addPublicTool(TauSelectorGNTauTight)

        TauGNTauTightWrapper = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                                 name               = "TauGNTauTightWrapper",
                                                                                 AsgSelectionTool   = TauSelectorGNTauTight,
                                                                                 StoreGateEntryName = "DFTauGNTauTight",
                                                                                 ContainerName      = kwargs["TauContainerName"]))
        TauAugmentationTools.append(TauGNTauTightWrapper)


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

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
    acc.merge(DiTauBuilderCfg(flags, name="DiTauLowPtBuilder", doLowPt=True))

    return acc

def AddDiTauIDDecorationCfg(flags, **kwargs):
    """Decorate ditau ID scores """

    acc = ComponentAccumulator()

    import DiTauRec.DiTauToolsConfig as DiTauTools

    diTauOnnxScoreCalculator = acc.popToolsAndMerge(DiTauTools.DiTauOnnxScoreCalculatorCfg(
            flags, 
            onnxModelPath                   = "TrigTauRec/00-11-02/dev/boosted_ditau_omni_model.onnx",
        ))

    acc.addPublicTool(diTauOnnxScoreCalculator)

    DiTauIDDecoratorWrapper = CompFactory.DerivationFramework.DiTauIDDecoratorWrapper
    DiTauIDDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

    DiTauIDDecoratorWrapper = DiTauIDDecoratorWrapper(name               = "DiTauIDDecoratorWrapper",
                                                      DiTauContainerName = kwargs['DiTauContainerName'],
                                                      DiTauOnnxDiscriminantTool = diTauOnnxScoreCalculator)

    acc.addPublicTool(DiTauIDDecoratorWrapper)
    acc.addEventAlgo(DiTauIDDecoratorKernel(name              = "DiTauIDDecorKernel",
                                            AugmentationTools = [DiTauIDDecoratorWrapper]))
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
        # evaluate GNTau score for v0prune model
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,0,applyLooseTrackSel=True)) )
        # evaluate GNTau score for v1trunc model
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,1,applyLooseTrackSel=True)) )
        # set WPs decision for v0prune model
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags,0)) )
        # set WPs decision for v1trunc model
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


def AddDiTauChargeDecoratorCfg(flags, **kwargs):
    """Decorate DiTau charge"""

    kwargs.setdefault("DiTauContainerName", "DiTauJets")
    kwargs.setdefault("prefix",           kwargs['DiTauContainerName'])

    acc = ComponentAccumulator()
   
    DiTauChargeDecorator = CompFactory.DerivationFramework.DiTauChargeDecorator
    DiTauChargeDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

    prefix = kwargs['prefix']
    diTauChargeDecorator = DiTauChargeDecorator(name               = f"{prefix}_DiTauChargeDecorator",
                                                DiTauContainerName = kwargs['DiTauContainerName'])
    acc.addPublicTool(diTauChargeDecorator)
    acc.addEventAlgo(DiTauChargeDecoratorKernel(name              = f"{prefix}_DiTauIDDecorKernel",
                                                AugmentationTools = [diTauChargeDecorator]))

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
    tools_after.append( acc.popToolsAndMerge(tauTools.TauJetRNNEvaluatorCfg(flags,applyLooseTrackSel=True)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorJetRNNCfg(flags)) )
    tools_after.append( acc.popToolsAndMerge(tauTools.TauEleRNNEvaluatorCfg(flags,applyLooseTrackSel=True )) )
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
