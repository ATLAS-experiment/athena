# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddTauAugmentationCfg(flags, wp="RNNVeryLoose", **kwargs):
    kwargs.setdefault("TauContainerName", "TauJets")
 
    acc = ComponentAccumulator()

    # tau selection relies on RNN electron veto, we must decorate the fixed eveto WPs before applying tau selection
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName=kwargs["TauContainerName"]))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg

    TauAugmentationTools = []

    config = {
      "RNNVeryLoose" : "TauAnalysisAlgorithms/tau_selection_veryloose_noeleid.conf",
      "RNNLoose"     : "TauAnalysisAlgorithms/tau_selection_loose_noeleid.conf",
      "RNNMedium"    : "TauAnalysisAlgorithms/tau_selection_medium_noeleid.conf",
      "RNNTight"     : "TauAnalysisAlgorithms/tau_selection_tight_noeleid.conf",
      
      "GNTauVeryLoose" : "TauAnalysisAlgorithms/tau_selection_gntau_veryloose_noeleid.conf",
      "GNTauLoose"     : "TauAnalysisAlgorithms/tau_selection_gntau_loose_noeleid.conf",
      "GNTauMedium"    : "TauAnalysisAlgorithms/tau_selection_gntau_medium_noeleid.conf",
      "GNTauTight"     : "TauAnalysisAlgorithms/tau_selection_gntau_tight_noeleid.conf",
    }

    TauSelector = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                           name = f'TauSelector{wp}',
                                                           ConfigPath = config[wp]))
    acc.addPublicTool(TauSelector)

    TauWrapper  = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                    name               = f"Tau{wp}Wrapper",
                                                                    AsgSelectionTool   = TauSelector,
                                                                    StoreGateEntryName = f"DFTau{wp}",
                                                                    ContainerName      = kwargs["TauContainerName"]))
    TauAugmentationTools.append(TauWrapper)

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation(f"Tau{wp}AugmentationKernel", AugmentationTools = TauAugmentationTools))
    return acc

def AddTauIDDecorationCfg(flags, **kwargs):
    """Decorate tau ID scores and working points"""

    #kwargs.setdefault("evetoFix",         True)
    #kwargs.setdefault("GNNTauID",         True)
    tauContainerKey = kwargs.setdefault("TauContainerName", "TauJets")
    #kwargs.setdefault("prefix",           kwargs['TauContainerName'])

    acc = ComponentAccumulator()

    import tauRec.TauToolHolder as tauTools
    tools = []
    doEvetoWP = False
    scoreNames = []
    WPNames = []

    #def cacheToolProperties(tool):
    #    doEvetoWP = tool.UseAbsEta
    #    scoreName = [tool.ScoreName] if tools[-1].ScoreName != "RNNEleScore" else []
    #    wpName = tool.DecorWPNames

    if kwargs.pop('evetoFix', True):
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNCfg(flags)) )
        # Cache tool properties
        doEvetoWP |= tools[-1].UseAbsEta
        # The original RNNEleScore should not be overriden
        if tools[-1].ScoreName != "RNNEleScore": scoreNames.append(tools[-1].ScoreName)
        scoreNames.append(tools[-1].NewScoreName)
        WPNames += tools[-1].DecorWPNames

    if kwargs.pop('GNNTauID', True):
        # vertex-corrected clusters must be rebuilt for tau ID
        tools.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
        # Add in GNTau!
        # evaluate GNTau score for v0prune model
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,0,applyLooseTrackSel=True)) )
        # evaluate GNTau score for v1trunc model
        tools.append( acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,1,applyLooseTrackSel=True)) )
        # set WPs decision for v0prune model
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags,0)) )
        # Cache tool properties
        doEvetoWP |= tools[-1].UseAbsEta
        # The original RNNEleScore should not be overriden
        if tools[-1].ScoreName != "RNNEleScore": scoreNames.append(tools[-1].ScoreName)
        scoreNames.append(tools[-1].NewScoreName)
        WPNames += tools[-1].DecorWPNames

        # set WPs decision for v1trunc model
        tools.append( acc.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags,1)) )
        # Cache tool properties
        doEvetoWP |= tools[-1].UseAbsEta
        # The original RNNEleScore should not be overriden
        if tools[-1].ScoreName != "RNNEleScore": scoreNames.append(tools[-1].ScoreName)
        scoreNames.append(tools[-1].NewScoreName)
        WPNames += tools[-1].DecorWPNames

    if tools:
        kwargs.setdefault("DoEvetoWP", doEvetoWP)
        kwargs.setdefault("ScoreDecorationKeys", scoreNames)
        kwargs.setdefault("WPDecorationKeys", WPNames)

        for tool in tools:
            acc.addPublicTool(tool)
        kwargs.setdefault("TauIDTools", tools)

        TauIDDecoratorWrapper = CompFactory.DerivationFramework.TauIDDecoratorWrapper
        TauIDDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

        prefix = kwargs.pop('prefix', tauContainerKey)
        tauIDDecoratorWrapper = TauIDDecoratorWrapper(name = f"{prefix}_TauIDDecoratorWrapper",
                                                      **kwargs)
        print("PXQW TauIDDecoratorsWrapper: " + str(tauIDDecoratorWrapper))
        acc.addPublicTool(tauIDDecoratorWrapper)
        acc.addEventAlgo(TauIDDecoratorKernel(name = f"{prefix}_TauIDDecorKernel",
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
