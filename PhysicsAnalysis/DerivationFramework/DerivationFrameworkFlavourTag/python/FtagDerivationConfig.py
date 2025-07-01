# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.BTagConfig import BTagAlgsCfg, GetTaggerTrainingMap
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg

import ParticleJetTools.ParentDecoratorConfig as pdc

PFLOW_JETS = 'AntiKt4EMPFlowJets'


def _addDepsByTaggername(cfgFlags, tagger: str) -> ComponentAccumulator:
    """
    Add additional algorithms based on the dirname of the network files.

    Parameters
    ----------
    cfgFlags : ConfigFlags
        The configuration flags for.
    tagger : str
        The name of the tagger.

    Returns
    -------
    ComponentAccumulator
        An accumulator containing the additional algorithms based on the dirname.
    """
    acc = ComponentAccumulator()
    if "GN2Xv02" in tagger:
        acc.merge(TrackLeptonDecorationCfg(cfgFlags))
    return acc


def HLTJetFTagDecorationCfg(cfgFlags):
    from ParticleJetTools.ParticleJetToolsConfig import getJetDeltaRFlavorLabelTool

    acc = ComponentAccumulator()

    jetDec = CompFactory.JetDecorationAlg(
        name='hltJetLabelingAlg', 
        JetContainer='HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf',
        Decorators=[getJetDeltaRFlavorLabelTool()]) 

    acc.addEventAlgo(jetDec)

    return acc

def BTagLargeRDecoration(cfgFlags, jet_col):

    jet_col_name_without_Jets = jet_col.replace('Jets', '')
    nnFiles = GetTaggerTrainingMap(cfgFlags, jet_col_name_without_Jets)

    # Doesn't need to be configurable at the moment
    trackContainer = 'GhostTrack'
    primaryVertexContainer = 'PrimaryVertices'
    variableRemapping = {'BTagTrackToJetAssociator': trackContainer}

    acc = ComponentAccumulator()
    acc.merge(BTagTrackAugmenterAlgCfg(
        cfgFlags,
        TrackCollection='InDetTrackParticles',
        PrimaryVertexCollectionName=primaryVertexContainer,
    ))

    for nnFile in nnFiles:
        # ugly string parsing to get the tagger name
        tagger_name = nnFile.split('/')[-3]
        # separate calse for JetCalibTools models
        if nnFile.split('/')[0] == "JetCalibTools":
            # not technically a tagger, but works in this code
            tagger_name = nnFile.split('_')[-2]

        acc.merge(_addDepsByTaggername(cfgFlags, tagger_name))

        acc.addEventAlgo(
            CompFactory.FlavorTagInference.JetTagDecoratorAlg(
                f'{jet_col}{tagger_name}JetTagAlg',
                container=jet_col,
                constituentContainer=trackContainer,
                decorator=CompFactory.FlavorTagInference.GNNTool(
                    tagger_name,
                    nnFile=nnFile,
                    variableRemapping=variableRemapping,
                    trackLinkType='IPARTICLE'
                ),
            )
        )

    return acc


def LegacyBTaggingCfg(cfgFlags, jet_col, pv_col='PrimaryVertices',
                           trackAugmenterPrefix=None):
    """
    Return a component accumulator which runs tagging on a single jet collection.
    """ 

    jet_col_name_without_Jets = jet_col.replace('Jets','')
    track_collection = _getTrackCollection(cfgFlags)
    input_muons = 'Muons'
    if cfgFlags.BTagging.Pseudotrack:
        input_muons = None

    acc = ComponentAccumulator()
    acc.merge(BTagTrackAugmenterAlgCfg(
        cfgFlags,
        TrackCollection=track_collection,
        PrimaryVertexCollectionName=pv_col,
        prefix=trackAugmenterPrefix
    ))

    # schedule tagging algorithms for this jet collection

    acc.merge(BTagAlgsCfg(
        inputFlags=cfgFlags,
        JetCollection=jet_col_name_without_Jets,
        nnList=GetTaggerTrainingMap(cfgFlags, jet_col_name_without_Jets),
        trackCollection=track_collection,
        primaryVertices=pv_col,
        muons=input_muons,
        AddedJetSuffix='Jets',
    ))

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
