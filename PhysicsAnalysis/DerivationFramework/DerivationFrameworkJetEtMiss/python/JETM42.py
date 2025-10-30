# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM42.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
logJETM42 = logging.getLogger('JETM42')

# Main algorithm config
def JETM42KernelCfg(flags, name='JETM42Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM42"""
    acc = ComponentAccumulator()
    
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        flags,
        TriggerListsHelper     = kwargs['TriggerListsHelper'],
    ))

    thinningToolsArgs = {
        'DiTauLowPtThinningToolName'          : "PHYSDiTauLowPtThinningTool",
    } 

    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg

    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))


    from JetRecConfig.JetRecConfig import JetRecCfg, getInputAlgs
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst, standardReco, stdInputExtDic as inpext
    from ROOT import xAODType

    #=======================================
    # CHS R = 0.4 422 jets
    #=======================================

    from JetRecConfig.JetDefinition import JetDefinition, JetInputConstitSeq, JetInputExternal
    from JetRecConfig.StandardSmallRJets import standardghosts, flavourghosts, truthmods, clustermods

    inpext["TC422"] = JetInputExternal("CaloTopoClusters422", xAODType.CaloCluster, algoBuilder= standardReco("CaloClusters"))
    inpext["TopoTowers"] = JetInputExternal("CaloCalAllTopoTowers", xAODType.CaloCluster, algoBuilder= standardReco("CaloClusters"))
    cst["EMTopo422"] = JetInputConstitSeq("EMTopo422", xAODType.CaloCluster, ["EM"],
                       "CaloTopoClusters422", "EMTopoClusters422", jetinputtype="EMTopo",
                       )
    cst["EMTopo422SK"] = JetInputConstitSeq("EMTopo422SK", xAODType.CaloCluster, ["EM","SK"],
                       "CaloTopoClusters422", "EMTopoClusters422SK", jetinputtype="EMTopo",
                       )
    cst["TopoTower"] = JetInputConstitSeq("TopoTower", xAODType.CaloCluster, ["EM"],
                       "CaloCalAllTopoTowers", "TopoTowers", jetinputtype="TopoTower",
                       )
    cst["TopoTowerSK"] = JetInputConstitSeq("TopoTowerSK", xAODType.CaloCluster, ["EM","SK"],
                       "CaloCalAllTopoTowers", "TopoTowersSK", jetinputtype="TopoTower",
                       )
    algs = getInputAlgs(cst["EMTopo422"], flags=flags)
    algs += getInputAlgs(cst["EMTopo422SK"], flags=flags)
    algs += getInputAlgs(cst["TopoTower"], flags=flags)
    algs += getInputAlgs(cst["TopoTowerSK"], flags=flags)

    AntiKt4EMTopo422 = JetDefinition("AntiKt",0.4,cst["EMTopo422"],
                              ghostdefs = standardghosts+["TrackLRT"]+flavourghosts,
                              modifiers = truthmods+clustermods+("Filter:15000","LArHVCorr",),
                              lock = True,
    )
    AntiKt4EMTopo422SK = JetDefinition("AntiKt",0.4,cst["EMTopo422SK"],
                              ghostdefs = standardghosts+["TrackLRT"]+flavourghosts,
                              modifiers = truthmods+clustermods+("Filter:15000","LArHVCorr",),
                              lock = True,
    )
    AntiKt4TopoTowers = JetDefinition("AntiKt",0.4,cst["TopoTower"],
                              ghostdefs = standardghosts+["TrackLRT"]+flavourghosts,
                              modifiers = truthmods+clustermods+("Filter:15000","LArHVCorr",),
                              lock = True,
    )
    AntiKt4TopoTowersSK = JetDefinition("AntiKt",0.4,cst["TopoTowerSK"],
                              ghostdefs = standardghosts+["TrackLRT"]+flavourghosts,
                              modifiers = truthmods+clustermods+("Filter:15000","LArHVCorr",),
                              lock = True,
    )

    jetList = [AntiKt4EMTopo422, AntiKt4EMTopo422SK, AntiKt4TopoTowers, AntiKt4TopoTowersSK]

    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []
    
    # thinning tools
    thinningTools = []

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def JETM42CoreCfg(flags, name, StreamName, TriggerListsHelper):

    acc = ComponentAccumulator()
    
    from DerivationFrameworkPhys.PHYS import PHYSCoreCfg
    acc.merge(PHYSCoreCfg(flags, name, StreamName = StreamName, TriggerListsHelper = TriggerListsHelper))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    JETM42SlimmingHelper = SlimmingHelper(name+"SlimmingHelper", flags=flags, NamesAndTypes = flags.Input.TypedCollections)

    JETM42SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloTopoClusters422", "CaloCalAllTopoTowers",
                                         "EMTopoClusters422","EMTopoClusters422SK","TopoTowers","TopoTowersSK",
                                      ]

    JETM42SlimmingHelper.AppendToDictionary.update({'EMTopoClusters422': 'xAOD::CaloClusterContainer',
                                                   'EMTopoClusters422Aux': 'xAOD::ShallowAuxContainer',
                                                   'EMTopoClusters422SK': 'xAOD::CaloClusterContainer',
                                                   'EMTopoClusters422SKAux': 'xAOD::ShallowAuxContainer',
                                                   'TopoTowers': 'xAOD::CaloClusterContainer',
                                                   'TopoTowersAux': 'xAOD::ShallowAuxContainer',
                                                   'TopoTowersSK': 'xAOD::CaloClusterContainer',
                                                   'TopoTowersSKAux': 'xAOD::ShallowAuxContainer',
                                                   })


    jetOutputList = ["AntiKt4EMTopo422Jets","AntiKt4EMTopo422SKJets","AntiKt4TopoTowerJets","AntiKt4TopoTowerSKJets"] 
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM42SlimmingHelper, jetOutputList, JETM42SlimmingHelper.SmartCollections)

    JETM42SlimmingHelper.ExtraVariables += [
        'PrimaryVertices.trackWeights',
        'HLT_ftfChargedParticleFlowObjectsAuxDyn.charge.chargedObjectLinks.chargedObjectWeights.eta.m.otherObjectLinks.otherObjectWeights.phi.pt.signalType.vertexType',
        'HLT_ftfNeutralParticleFlowObjectsAuxDyn.charge.chargedObjectLinks.chargedObjectWeights.eta.m.otherObjectLinks.otherObjectWeights.phi.pt.signalType.vertexType',
    ]

    # Pass through all trigger content
    JETM42SlimmingHelper.IncludeFullTriggerEDMLevel = "ESD"

    # Output stream    
    JETM42ItemList = JETM42SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name, ItemList=JETM42ItemList, AcceptAlgs=[name+"Kernel"]))

    return acc


def JETM42Cfg(flags):
    acc = ComponentAccumulator()
    
    # the name_tag has to consistent between KernelCfg and CoreCfg
    JETM42_name_tag = 'JETM42'

    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM42TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(JETM42KernelCfg(flags,
        name= JETM42_name_tag + "Kernel", 
        StreamName = 'StreamDAOD_'+JETM42_name_tag,
        TriggerListsHelper = JETM42TriggerListsHelper,
        ))
    
    # PHYS content
    acc.merge(JETM42CoreCfg(flags, 
        name=JETM42_name_tag,
        StreamName = 'StreamDAOD_'+JETM42_name_tag,
        TriggerListsHelper = JETM42TriggerListsHelper,
        ))

    return acc


