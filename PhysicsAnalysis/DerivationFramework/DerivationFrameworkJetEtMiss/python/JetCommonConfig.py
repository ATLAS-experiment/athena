# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Contains the configuration for common jet reconstruction + decorations
# used in analysis DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetCommonCfg(ConfigFlags):
    """Main config for jet reconstruction and decorations"""

    acc = ComponentAccumulator()

    acc.merge(StandardJetsInDerivCfg(ConfigFlags))
    if "McEventCollection#GEN_EVENT" not in ConfigFlags.Input.TypedCollections:
        acc.merge(AddBadBatmanCfg(ConfigFlags))
    acc.merge(AddDistanceInTrainCfg(ConfigFlags))
    acc.merge(AddSidebandEventShapeCfg(ConfigFlags))
    acc.merge(AddEventCleanFlagsCfg(ConfigFlags))

    return acc


def StandardJetsInDerivCfg(ConfigFlags):
    """Jet reconstruction needed for PHYS/PHYSLITE"""

    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo,AntiKt4EMPFlow
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopoTrimmed,AntiKt10UFOCSSKSoftDrop
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.JetConfigFlags import jetInternalFlags

    acc = ComponentAccumulator()

    AntiKt4EMTopo_deriv = AntiKt4EMTopo.clone(
        modifiers = AntiKt4EMTopo.modifiers+("JetPtAssociation","QGTagging")
    )

    AntiKt4EMPFlow_deriv = AntiKt4EMPFlow.clone(
        modifiers = AntiKt4EMPFlow.modifiers+("JetPtAssociation","QGTagging","fJVT","NNJVT","CaloEnergiesClus","JetPileupLabel")
    )

    jetList = [AntiKt4EMTopo_deriv, AntiKt4EMPFlow_deriv,
               AntiKt10LCTopoTrimmed,AntiKt10UFOCSSKSoftDrop]

    jetInternalFlags.isRecoJob = True

    for jd in jetList:
        acc.merge(JetRecCfg(ConfigFlags,jd))

    return acc

def AddBadBatmanCfg(ConfigFlags):
    """Add bad batman decoration for events with large EMEC-IW noise"""

    acc = ComponentAccumulator()

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkJetEtMiss.JetToolConfig import BadBatmanToolCfg
    badBatmanTool = acc.getPrimaryAndMerge(BadBatmanToolCfg(ConfigFlags))
    acc.addEventAlgo(CommonAugmentation("BadBatmanAugmentation", AugmentationTools = [badBatmanTool]))

    return acc

def AddDistanceInTrainCfg(ConfigFlags):
    """Add distance in train information to EventInfo"""
    from DerivationFrameworkJetEtMiss.JetToolConfig import DistanceInTrainToolCfg

    acc = ComponentAccumulator()

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    distanceInTrainTool = acc.getPrimaryAndMerge(DistanceInTrainToolCfg(ConfigFlags))
    acc.addEventAlgo(CommonAugmentation("DistanceInTrainAugmentation", AugmentationTools = [distanceInTrainTool]))

    return acc

def AddSidebandEventShapeCfg(ConfigFlags):
    """Special rho definitions for PFlow jets"""
    from JetRecConfig.JetRecConfig import getInputAlgs,getConstitPJGAlg,reOrderAlgs
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetInputConfig import buildEventShapeAlg

    acc = ComponentAccumulator()

    constit_algs = getInputAlgs(cst.GPFlow, flags=ConfigFlags)
    constit_algs, ca = reOrderAlgs( [a for a in constit_algs if a is not None])

    acc.merge(ca)
    for a in constit_algs:
        acc.addEventAlgo(a)

    #Sideband definition
    acc.addEventAlgo(getConstitPJGAlg(cst.GPFlow, suffix='PUSB'))
    acc.addEventAlgo(buildEventShapeAlg(cst.GPFlow, '', suffix = 'PUSB' ))

    #New "sideband" definition when using CHS based on TTVA
    acc.addEventAlgo(getConstitPJGAlg(cst.GPFlow, suffix='Neut'))
    acc.addEventAlgo(buildEventShapeAlg(cst.GPFlow, '', suffix = 'Neut' ))

    return acc

def AddJvtDecorationAlgCfg(ConfigFlags, algName = "JvtPassDecorAlg", **kwargs):
    acc = ComponentAccumulator()
    # Decorate if jet passed JVT criteria
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtEffToolCfg
    passJvtTool = acc.popToolsAndMerge(getJvtEffToolCfg(ConfigFlags, 'AntiKt4EMTopo'))
    passJvtTool.PassJVTKey = "AntiKt4EMTopoJets.DFCommonJets_passJvt"
    passJvtTool.SuppressOutputDependence = False
    kwargs.setdefault("Decorators", [passJvtTool])
    kwargs.setdefault("JetContainer", "AntiKt4EMTopoJets")
    acc.addEventAlgo(CompFactory.JetDecorationAlg(algName, **kwargs), primary = True)
    return acc

def AddEventCleanFlagsCfg(ConfigFlags, workingPoints = ['Loose', 'Tight', 'LooseLLP']):
    """Add event cleaning flags"""

    acc = ComponentAccumulator()
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags))
    

    from DerivationFrameworkTau.TauCommonConfig import AddTauAugmentationCfg
    acc.merge(AddTauAugmentationCfg(ConfigFlags, prefix="JetCommon", doLoose=True))

    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg
    outputLabel = 'DFCommonJets_passOR'
    bJetLabel = '' #default
    tauLabel = 'DFTauLoose'
    orTool = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,outputLabel=outputLabel,bJetLabel=bJetLabel))
    algOR = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg',
                                                OverlapLabel=outputLabel,
                                                OverlapRemovalTool=orTool,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel)
    acc.addEventAlgo(algOR)

    # Explictly lock the decorations produced by overlap removal.
    # This is problematic to do inside the overlap removal algorithm
    # itself, because we schedule two of them above --- the second
    # overwriting most of the decorations produced by the first.
    # This is not MT-safe.
    lockOR = CompFactory.DerivationFramework.LockDecorations \
        ('OverlapRemovalLockDecorAlg',
         Decorations = [
             'Electrons.selected',
             'Electrons.' + outputLabel,
             'Muons.selected',
             'Muons.' + outputLabel,
             'Photons.selected',
             'Photons.' + outputLabel,
             'AntiKt4EMPFlowJets.selected',
             'AntiKt4EMPFlowJets.' + outputLabel,
             'AntiKt4EMTopoJets.selected',
             'AntiKt4EMTopoJets.' + outputLabel,
             'TauJets.selected',
             'TauJets.' + outputLabel,
         ])
    acc.addEventAlgo(lockOR)

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonJetDrToolCfg
    muonJetDrTool = acc.getPrimaryAndMerge(MuonJetDrToolCfg(ConfigFlags, "MuonJetDrTool"))
    acc.addEventAlgo(CommonAugmentation("DFCommonMuonsKernel2", AugmentationTools = [muonJetDrTool]))

    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg,JetCleaningToolCfg
    
    supportedWPs = ['Loose', 'Tight', 'LooseLLP', 'VeryLooseLLP', 'SuperLooseLLP']
    prefix = "DFCommonJets_"

    for wp in workingPoints:
        if wp not in supportedWPs:
            continue
            
        cleaningLevel = wp + 'Bad'
        # LLP WPs have a slightly different name format
        if 'LLP' in wp:
            cleaningLevel = wp.replace('LLP', 'BadLLP')

        jetCleaningTool = acc.popToolsAndMerge(JetCleaningToolCfg(ConfigFlags, 'JetCleaningTool_'+cleaningLevel, 'AntiKt4EMTopoJets', cleaningLevel, False))
        acc.addPublicTool(jetCleaningTool)

        ecTool = acc.popToolsAndMerge(EventCleaningToolCfg(ConfigFlags,'EventCleaningTool_' + wp, cleaningLevel))
        ecTool.JetCleanPrefix = prefix
        ecTool.JetContainer = "AntiKt4EMTopoJets"
        ecTool.JetCleaningTool = jetCleaningTool
        acc.addPublicTool(ecTool)

        eventCleanAlg = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp,
                                                         EventCleaningTool=ecTool,
                                                         JetCollectionName="AntiKt4EMTopoJets",
                                                         EventCleanPrefix=prefix,
                                                         CleaningLevel=cleaningLevel,
                                                         doEvent = ('Loose' in wp)) # Only store event-level flags for Loose and LooseLLP
        acc.addEventAlgo(eventCleanAlg)

    return acc


def addJetsToSlimmingTool(slimhelper,contentlist,smartlist=[]):
    for item in contentlist:
        if item not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({item:'xAOD::JetContainer',
                                                  item+"Aux":'xAOD::JetAuxContainer'})
        if item in smartlist:
            slimhelper.SmartCollections.append(item)
        else:
            slimhelper.AllVariables.append(item)


##################################################################
# Helper to add origin corrected clusters to output
##################################################################
def addOriginCorrectedClustersToSlimmingTool(slimhelper,writeLC=False,writeEM=False):

    slimhelper.ExtraVariables.append('CaloCalTopoClusters.calE.calEta.calPhi.calM')

    if writeLC:
        if "LCOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({"LCOriginTopoClusters":'xAOD::CaloClusterContainer',
                                                  "LCOriginTopoClustersAux":'xAOD::ShallowAuxContainer'})
            slimhelper.ExtraVariables.append('LCOriginTopoClusters.calEta.calPhi')

    if writeEM:
        if "EMOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({"EMOriginTopoClusters":'xAOD::CaloClusterContainer',
                                                  "EMOriginTopoClustersAux":'xAOD::ShallowAuxContainer'})
            slimhelper.ExtraVariables.append('EMOriginTopoClusters.calE.calEta.calPhi')
