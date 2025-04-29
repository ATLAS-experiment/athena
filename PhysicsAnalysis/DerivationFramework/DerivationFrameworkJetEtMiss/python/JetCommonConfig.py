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

    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo,AntiKt4EMPFlow,AntiKtVR30Rmax4Rmin02PV0Track
    from JetRecConfig.StandardLargeRJets import AntiKt10UFOCSSKSoftDrop
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc = ComponentAccumulator()

    AntiKt4EMTopo_deriv = AntiKt4EMTopo.clone(
        modifiers = AntiKt4EMTopo.modifiers+("JetPtAssociation","QGTagging")
    )

    AntiKt4EMPFlow_deriv = AntiKt4EMPFlow.clone(
        modifiers = AntiKt4EMPFlow.modifiers+("JetPtAssociation","QGTagging","fJVT","NNJVT","CaloEnergiesClus","JetPileupLabel","qgtransformer")
    )

    jetList = [AntiKt4EMTopo_deriv, AntiKt4EMPFlow_deriv,
               AntiKtVR30Rmax4Rmin02PV0Track,
               AntiKt10UFOCSSKSoftDrop]

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

def AddJvtDecorationAlgCfg(ConfigFlags, algName = "JvtPassDecorAlg", jetContainer='AntiKt4EMTopo', **kwargs):
    acc = ComponentAccumulator()
    # Decorate if jet passed JVT criteria
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtEffToolCfg

    passJvtTool = acc.popToolsAndMerge(getJvtEffToolCfg(ConfigFlags, jetContainer))
    passJvtTool.PassJVTKey = "{}Jets.DFCommonJets_passJvt".format(jetContainer)
    passJvtTool.SuppressOutputDependence = False
    kwargs.setdefault("Decorators", [passJvtTool])
    kwargs.setdefault("JetContainer", "{}Jets".format(jetContainer))
    acc.addEventAlgo(CompFactory.JetDecorationAlg(algName, **kwargs), primary = True)
    return acc

def AddEventCleanFlagsCfg(ConfigFlags, workingPoints = ['Loose', 'Tight', 'LooseLLP']):
    """Add event cleaning flags"""

    acc = ComponentAccumulator()
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags, algName="JvtPassDecorAlg_EMTopo", jetContainer='AntiKt4EMTopo'))
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags, algName="JvtPassDecorAlg", jetContainer='AntiKt4EMPFlow'))

    from DerivationFrameworkTau.TauCommonConfig import AddTauAugmentationCfg
    acc.merge(AddTauAugmentationCfg(ConfigFlags, prefix="JetCommon", doRNNLoose=True))

    # The overlap removal algorithm presents difficulties.
    # It leaves decorations unlocked.
    # Further, configurations may schedule multiple overlap removal algorithms,
    # sometimes outside of this file, which then overwrite each other's
    # results.  So to get decoration locking to work properly, we need
    # to first group all the event cleaning algorithms together,
    # immediately followed by LockDecorations algorithms to lock the
    # decorations produced by overlap.  To accomplish this, we create
    # two sequences, one for event cleaning and one for decoration locking
    # and add the algorithms there.  By default, these sequences will
    # be scheduled at the current point in the global algorithm sequence,
    # but if another fragment adds additional overlap removal, it may need
    # to move the sequences later.
    # All this is of course not MT-safe.
    acc.addSequence(CompFactory.AthSequencer('EventCleanSeq', Sequential=True))
    acc.addSequence(CompFactory.AthSequencer('EventCleanLockSeq', Sequential=True))

    # Overlap for EMTopo
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg
    outputLabel_legacy = 'DFCommonJets_passOR'
    bJetLabel = '' #default
    tauLabel = 'DFTauRNNLoose'
    orTool_legacy = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,outputLabel=outputLabel_legacy,bJetLabel=bJetLabel))
    algOR_legacy = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg_EMTopo',
                                                JetKey="AntiKt4EMTopoJets",
                                                OverlapLabel=outputLabel_legacy,
                                                OverlapRemovalTool=orTool_legacy,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel
                                                )
    acc.addEventAlgo(algOR_legacy, 'EventCleanSeq')

    # Overlap for EMPFlow
    outputLabel = 'DFCommonJets_passOR'
    orTool = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,outputLabel=outputLabel,bJetLabel=bJetLabel))
    algOR = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg',
                                                OverlapLabel=outputLabel,
                                                OverlapRemovalTool=orTool,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel)
    acc.addEventAlgo(algOR, 'EventCleanSeq')

    # Explictly lock the decorations produced by overlap removal.
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
    acc.addEventAlgo(lockOR, 'EventCleanLockSeq')

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonJetDrToolCfg
    muonJetDrTool = acc.getPrimaryAndMerge(MuonJetDrToolCfg(ConfigFlags, "MuonJetDrTool"))
    acc.addEventAlgo(CommonAugmentation("DFCommonMuonsKernel2", AugmentationTools = [muonJetDrTool]), 'EventCleanSeq')

    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg,JetCleaningToolCfg
    
    supportedWPs = ['Loose', 'Tight', 'LooseLLP', 'VeryLooseLLP', 'SuperLooseLLP']
    prefix = "DFCommonJets_"
    evt_lvl_suppWPs_PFlow = ['LooseBad', 'TightBad']

    for wp in workingPoints:
        if wp not in supportedWPs:
            continue
            
        cleaningLevel = wp + 'Bad'
        # LLP WPs have a slightly different name format
        if 'LLP' in wp:
            cleaningLevel = wp.replace('LLP', 'BadLLP')
        
        # Add support for TightBad event flag as well
        doEvent_PFlow=False
        for evt_swp in evt_lvl_suppWPs_PFlow:
            if evt_swp == cleaningLevel: 
                doEvent_PFlow=True
                break

        doEvent_EMTopo=False
        if 'Loose' in cleaningLevel: 
            doEvent_EMTopo=True

        ## for EMTopo (Legacy), also support for LLPs
        if doEvent_EMTopo:
            jetCleaningTool_legacy = acc.popToolsAndMerge(JetCleaningToolCfg(
                    ConfigFlags, 'JetCleaningTool_'+cleaningLevel+'_EMTopo',
                    'AntiKt4EMTopo', cleaningLevel, False))
            acc.addPublicTool(jetCleaningTool_legacy)
            ecTool_legacy = acc.popToolsAndMerge(EventCleaningToolCfg(
                    ConfigFlags,'EventCleaningTool_'+wp+'_EMTopo', cleaningLevel))
            ecTool_legacy.JetCleanPrefix = prefix
            ecTool_legacy.JetContainer = "AntiKt4EMTopoJets"
            ecTool_legacy.JetCleaningTool = jetCleaningTool_legacy
            acc.addPublicTool(ecTool_legacy)
    
            eventCleanAlg_legacy = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp+'_EMTopo',
                                                             EventCleaningTool=ecTool_legacy,
                                                             JetCollectionName="AntiKt4EMTopoJets",
                                                             EventCleanPrefix=prefix,
                                                             CleaningLevel=cleaningLevel,
                                                             doEvent=True) # Only store event-level flags for Loose and LooseLLP
            acc.addEventAlgo(eventCleanAlg_legacy, 'EventCleanSeq')

        ## For PFlow
        if doEvent_PFlow:
            jetCleaningTool = acc.popToolsAndMerge(JetCleaningToolCfg(
                    ConfigFlags, 'JetCleaningTool_'+cleaningLevel,
                    'AntiKt4EMPFlowJets', cleaningLevel, False))
            acc.addPublicTool(jetCleaningTool)
    
            ecTool = acc.popToolsAndMerge(EventCleaningToolCfg(ConfigFlags,'EventCleaningTool_' + wp, cleaningLevel))
            ecTool.JetCleanPrefix = prefix
            ecTool.JetContainer = "AntiKt4EMPFlowJets"
            ecTool.JetCleaningTool = jetCleaningTool
            acc.addPublicTool(ecTool)
    
            eventCleanAlg = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp,
                                                             EventCleaningTool=ecTool,
                                                             JetCollectionName="AntiKt4EMPFlowJets",
                                                             EventCleanPrefix=prefix,
                                                             CleaningLevel=cleaningLevel,
                                                             doEvent=True) # for PFlow we use Loose and Tight
            acc.addEventAlgo(eventCleanAlg, 'EventCleanSeq')

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
            slimhelper.ExtraVariables.append('LCOriginTopoClusters.calEta.calPhi.originalObjectLink')

    if writeEM:
        if "EMOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({"EMOriginTopoClusters":'xAOD::CaloClusterContainer',
                                                  "EMOriginTopoClustersAux":'xAOD::ShallowAuxContainer'})
            slimhelper.ExtraVariables.append('EMOriginTopoClusters.calE.calEta.calPhi.originalObjectLink')
