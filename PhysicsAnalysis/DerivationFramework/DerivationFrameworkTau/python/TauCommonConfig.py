# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TauCPContentCfg(flags):
    """TauJets CP content, extended with the TausRUs heads when they are run."""
    from DerivationFrameworkTau.TauJetsCPContent import TauJetsCPContent
    if not flags.Tau.doTausRUs:
        return TauJetsCPContent

    import tauRec.TauToolHolder as tauTools
    # The per-track head is decorated on the TauTracks, not on the taus, so the
    # two lists go on different aux items.
    extraVars = {
        "TauJetsAux.": ".".join(tauTools.TausRUsDecorationNames()),
        "TauTracksAux.": ".".join(tauTools.TausRUsTrackDecorationNames()),
    }
    content = []
    for item in TauJetsCPContent:
        for prefix, extra in extraVars.items():
            if item.startswith(prefix):
                item += "." + extra
        content.append(item)
    return content

def AddTauAugmentationCfg(flags, wp="GNTauVeryLoose", **kwargs):
    kwargs.setdefault("TauContainerName", "TauJets")

    acc = ComponentAccumulator()

    # tau selection relies on RNN electron veto, we must decorate the fixed eveto WPs before applying tau selection
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName=kwargs["TauContainerName"]))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg

    TauAugmentationTools = []

    config = {
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

def AddTauMassDecoratorCfg(flags, **kwargs):
    """Decorate/overwrite the tau visible mass"""

    kwargs.setdefault("TauContainerName", "TauJets")
    kwargs.setdefault("prefix",           kwargs['TauContainerName'])

    acc = ComponentAccumulator()

    #Rename existing decorations in input file, such that we can create new ones.
    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge(InputRenameCfg("xAOD::TauJetContainer", kwargs['TauContainerName'], "old_"+kwargs['TauContainerName']))
    acc.merge(InputRenameCfg("xAOD::TauJetAuxContainer", kwargs['TauContainerName']+"Aux.", "old_"+kwargs['TauContainerName']+"Aux."))

    acc.addEventAlgo(CompFactory.DerivationFramework.TauMassDecorator(name               = kwargs['TauContainerName']+"_MassDecorator",
                                                                      TauOutputName      = kwargs['TauContainerName'],
                                                                      TauInputName       = "old_"+kwargs['TauContainerName']))

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
    # The vertex-corrected clusters are consumed by more than one network below,
    # but must only be rebuilt once.
    doVertexedClusters = False

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
        doVertexedClusters = True
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

    if kwargs.pop('TausRUs', flags.Tau.doTausRUs):
        if not doVertexedClusters:
            tools.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
            doVertexedClusters = True
        # Give the container a unique name for running
        tools.append( acc.popToolsAndMerge(tauTools.TausRUsEvaluatorCfg(
            flags,
            name=f"{tauContainerKey}_TausRUs",
            tauTrackContainerName=tauContainerKey.replace("TauJets", "TauTracks"))) )
        # The tools run on a shallow copy of the taus, so every tau decoration
        # has to be copied back, which is what the score list is. The per-track
        # decorations need no copy-back: the tracks the shallow copy links to
        # are the originals.
        scoreNames += tauTools.TausRUsDecorationNames()

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


def AddTauTESCompatibilityDecorationCfg(flags, **kwargs):
    """Decorate taus with a flag to check if Calo and MVA TES are compatible """

    acc = ComponentAccumulator()

    import tauRec.TauToolHolder as tauTools

    tauCombinedTESTool = acc.popToolsAndMerge(tauTools.TauCombinedTESCfg(
            flags,
            ))

    acc.addPublicTool(tauCombinedTESTool)
    kwargs.setdefault("TauCombinedTESTool", tauCombinedTESTool)
    kwargs.setdefault("TauContainerName", "TauJets")
    
    prefix = kwargs["TauContainerName"]
    TauCombinedTESWrapper = CompFactory.DerivationFramework.TauCombinedTESWrapper
    TauCombinedTESKernel = CompFactory.DerivationFramework.CommonAugmentation
    
    TauCombinedTESWrapper = TauCombinedTESWrapper( name = f"{prefix}_TauCombinedTES", **kwargs )                                     
    acc.addPublicTool(TauCombinedTESWrapper)
    acc.addEventAlgo(TauCombinedTESKernel(name              = f"{prefix}_TauCombinedTESKernel",
                                          AugmentationTools = [TauCombinedTESWrapper]))
    return acc
    

# Attach displaced Tau ID scores
def AddTauIDDisplacedDecorationCfg(flags, **kwargs):
    """Decorate displaced tau ID scores and working points. Follows AddTauIDDisplacedDecorationCfg()"""
    tauContainerKey = kwargs.setdefault("TauContainerName", "TauJetsLRT")

    flags_TauLRT = flags.cloneAndReplace("Tau.ActiveConfig", "Tau.TauLRT")

    acc = ComponentAccumulator()

    scoreNames = []

    import tauRec.TauToolHolder as tauTools
    tool_prompt = acc.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags,0,applyLooseTrackSel=True))
    scoreNames += ["GNTauScore_v0prune"]

    tool_displaced =  acc.popToolsAndMerge(tauTools.TauGNNDisplacedEvaluatorCfg(flags_TauLRT, tauContainerName=tauContainerKey))
    scoreNames += ["GNdTauScore", "GNdTauProbTau",  "GNdTauProbJet"]

    kwargs.setdefault("ScoreDecorationKeys", scoreNames)
    kwargs.setdefault("WPDecorationKeys", [])

    acc.addPublicTool(tool_prompt)
    acc.addPublicTool(tool_displaced)
    kwargs.setdefault("TauIDTools", [tool_prompt, tool_displaced])

    tauIDDecoratorWrapper = CompFactory.DerivationFramework.TauIDDecoratorWrapper(
        name = f"{tauContainerKey}_TauIDDisplacedDecoratorWrapper",
        **kwargs,
    )
    acc.addPublicTool(tauIDDecoratorWrapper)

    prefix = kwargs.pop('prefix', tauContainerKey)
    acc.addEventAlgo(
        CompFactory.DerivationFramework.CommonAugmentation(
            name = f"{prefix}_TauDisplacedIDDecorKernel",
            AugmentationTools = [tauIDDecoratorWrapper],
        )
    )
    return acc

# TauJets_MuonRM steering
def AddMuonRemovalTauAODReRecoAlgCfg(flags, **kwargs):
    """Configure the MuonRM AOD tau building"""

    acc = ComponentAccumulator()
    inputTauJets = kwargs.setdefault("Key_tauContainer", "TauJets")
    kwargs.setdefault("Key_tauOutputContainer", "TauJets_MuonRM")
    kwargs.setdefault("Key_pi0OutputContainer", "TauFinalPi0s_MuonRM")
    kwargs.setdefault("Key_neutralPFOOutputContainer", "TauNeutralParticleFlowObjects_MuonRM")
    kwargs.setdefault("Key_chargedPFOOutputContainer", "TauChargedParticleFlowObjects_MuonRM")
    kwargs.setdefault("Key_hadronicPFOOutputContainer", "TauHadronicParticleFlowObjects_MuonRM")
    kwargs.setdefault("Key_tauTrackOutputContainer", "TauTracks_MuonRM")
    kwargs.setdefault("Key_vertexOutputContainer", "TauSecondaryVertices_MuonRM")

    # get tools from holder
    import tauRec.TauToolHolder as tauTools
    if "modificationTools" not in kwargs:
        tools_mod = []
        tools_mod.append( acc.popToolsAndMerge(tauTools.TauAODMuonRemovalCfg(flags)) )
        for tool in tools_mod:
            tool.inAOD = True
        kwargs.setdefault("modificationTools", tools_mod)

    if "officialTools" not in kwargs:
        tools_after = []
        tools_after.append( acc.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )
        tools_after.append( acc.popToolsAndMerge(tauTools.TauTrackRNNClassifierCfg(flags)) )
        tools_after.append( acc.popToolsAndMerge(tauTools.EnergyCalibrationLCCfg(flags,force_zero_mass=True)) )
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
        for tool in tools_after:
            tool.inAOD = True
        kwargs.setdefault("officialTools", tools_after)

    if flags.Input.isMC:
        kwargs.setdefault("ExtraInputs",
                        [ ( 'xAOD::TauJetContainer' , f"StoreGateSvc+{inputTauJets}.truthJetLink" ),
                            ( 'xAOD::TauJetContainer' , f"StoreGateSvc+{inputTauJets}.truthParticleLink" ) ] )
    TauAODRunnerAlg=CompFactory.getComp("TauAODRunnerAlg")
    myTauAODRunnerAlg = TauAODRunnerAlg(
        name = "MuonRemovalTauAODReRecoAlg",
        **kwargs
    )
    acc.addEventAlgo(myTauAODRunnerAlg)
    return acc


def TauThinningCfg(flags, name, **kwargs):
    """configure tau thinning"""

    acc = ComponentAccumulator()
    TauThinningTool = CompFactory.DerivationFramework.TauThinningTool
    acc.addPublicTool(TauThinningTool(name, **kwargs), primary=True)
    return acc
