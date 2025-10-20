# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

import ParticleJetTools.ParentDecoratorConfig as pdc

PFLOW_JETS = 'AntiKt4EMPFlowJets'


def HLTJetFTagDecorationCfg(cfgFlags):
    from ParticleJetTools.ParticleJetToolsConfig import getJetDeltaRFlavorLabelTool

    acc = ComponentAccumulator()

    jetDec = CompFactory.JetDecorationAlg(
        name='hltJetLabelingAlg', 
        JetContainer='HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf',
        Decorators=[getJetDeltaRFlavorLabelTool()]) 

    acc.addEventAlgo(jetDec)

    return acc


def trackTruthDecorator(cfgFlags) -> ComponentAccumulator:
    """Decorate tracks with detailed truth information."""
    acc = ComponentAccumulator()
    if not cfgFlags.Input.isMC:
        return acc

    from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
        InDetTrackTruthOriginToolCfg,
    )
    trackTruthOriginTool = acc.popToolsAndMerge(InDetTrackTruthOriginToolCfg(cfgFlags))
    acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TruthParticleDecoratorAlg(
        'TruthParticleDecoratorAlg',
        trackTruthOriginTool=trackTruthOriginTool
    ))
    acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TrackTruthDecoratorAlg(
        'TrackTruthDecoratorAlg',
        trackContainer=_getTrackCollection(cfgFlags),
        trackTruthOriginTool=trackTruthOriginTool,
        truthLeptonTool=CompFactory.TruthClassificationTool("TruthClassificationTool")
    ))

    return acc


def _getTrackCollection(cfgFlags):
    if cfgFlags.BTagging.Pseudotrack:
        return 'InDetPseudoTrackParticles'
    return 'InDetTrackParticles'


def ParentDecoratorCfg(flags, prefix="", **kwargs):
    cfg = ComponentAccumulator()
    cfg.merge(pdc.HiggsParentDecoratorCfg(
        flags, name=prefix + "HiggsParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.ZParentDecoratorCfg(
        flags, name=prefix + "ZParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.ScalarParentDecoratorCfg(
        flags, name=prefix + "ScalarParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.TopParentDecoratorCfg(
        flags, name=prefix + "TopParentDecoratorAlg", **kwargs))
    return cfg


# Valerio's magic hacks for emtopo
def RenameInputContainerEmTopoHacksCfg(suffix):
    acc = ComponentAccumulator()

    #Delete BTagging container read from input ESD
    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    AddressRemappingSvc = AddressRemappingSvc("AddressRemappingSvc")
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.BTagTrackToJetAssociator->AntiKt4EMTopoJets.BTagTrackToJetAssociator_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.JFVtx->AntiKt4EMTopoJets.JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.SecVtx->AntiKt4EMTopoJets.SecVtx_' + suffix]

    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMTopoJets.btaggingLink->AntiKt4EMTopoJets.btaggingLink_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingContainer#BTagging_AntiKt4EMTopo->BTagging_AntiKt4EMTopo_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingAuxContainer#BTagging_AntiKt4EMTopoAux.->BTagging_AntiKt4EMTopo_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexContainer#BTagging_AntiKt4EMTopoSecVtx->BTagging_AntiKt4EMTopoSecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexAuxContainer#BTagging_AntiKt4EMTopoSecVtxAux.->BTagging_AntiKt4EMTopoSecVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexContainer#BTagging_AntiKt4EMTopoJFVtx->BTagging_AntiKt4EMTopoJFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexAuxContainer#BTagging_AntiKt4EMTopoJFVtxAux.->BTagging_AntiKt4EMTopoJFVtx_' + suffix+"Aux."]
    acc.addService(AddressRemappingSvc)
    acc.addService(ProxyProviderSvc(ProviderNames = [ "AddressRemappingSvc" ]))
    return acc

# Valerio's magic hacks for pflow
def RenameInputContainerEmPflowHacksCfg(suffix):
    acc = ComponentAccumulator()

    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    AddressRemappingSvc = AddressRemappingSvc("AddressRemappingSvc")
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.BTagTrackToJetAssociator->AntiKt4EMPFlowJets.BTagTrackToJetAssociator_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.JFVtx->AntiKt4EMPFlowJets.JFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.SecVtx->AntiKt4EMPFlowJets.SecVtx_' + suffix]

    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::JetAuxContainer#AntiKt4EMPFlowJets.btaggingLink->AntiKt4EMPFlowJets.btaggingLink_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingContainer#BTagging_AntiKt4EMPFlow->BTagging_AntiKt4EMPFlow_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTaggingAuxContainer#BTagging_AntiKt4EMPFlowAux.->BTagging_AntiKt4EMPFlow_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexContainer#BTagging_AntiKt4EMPFlowSecVtx->BTagging_AntiKt4EMPFlowSecVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::VertexAuxContainer#BTagging_AntiKt4EMPFlowSecVtxAux.->BTagging_AntiKt4EMPFlowSecVtx_' + suffix+"Aux."]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexContainer#BTagging_AntiKt4EMPFlowJFVtx->BTagging_AntiKt4EMPFlowJFVtx_' + suffix]
    AddressRemappingSvc.TypeKeyRenameMaps += ['xAOD::BTagVertexAuxContainer#BTagging_AntiKt4EMPFlowJFVtxAux.->BTagging_AntiKt4EMPFlowJFVtx_' + suffix+"Aux."]
    acc.addService(AddressRemappingSvc)
    acc.addService(ProxyProviderSvc(ProviderNames = [ "AddressRemappingSvc" ]))
    return acc
